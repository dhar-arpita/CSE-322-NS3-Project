
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/flow-monitor-module.h"

#include <fstream>
#include <iostream>
#include <cmath>
#include <set>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AodvAttackSim");

int main(int argc, char *argv[])
{
    uint32_t    nNodes     = 50;
    std::string attack     = "none";
    std::string defense    = "none";
    uint32_t    nMalicious = 1;
    double      dropProb   = 0.5;
    double      speed      = 5.0;
    double      simTime    = 500.0;    
    uint32_t    seed       = 30;
    uint32_t    mbdpK      = 4;

    CommandLine cmd;
    cmd.AddValue("nNodes",     "Number of nodes (default 50)",                 nNodes);
    cmd.AddValue("attack",     "Attack: none/blackhole/smartblack/grayhole/smartgray", attack);
    cmd.AddValue("defense",    "Defense: none/ids/mbdp",                       defense);
    cmd.AddValue("nMalicious", "Number of malicious nodes (1,2,3...)",         nMalicious);
    cmd.AddValue("dropProb",   "Grayhole drop probability (0.0-1.0)",         dropProb);
    cmd.AddValue("speed",      "Max node speed m/s",                           speed);
    cmd.AddValue("simTime",    "Simulation time seconds",                      simTime);
    cmd.AddValue("seed",       "RNG seed",                                     seed);
    cmd.AddValue("mbdpK",      "MBDP-AODV: replies to collect (default 4)",   mbdpK);
    cmd.Parse(argc, argv);

    RngSeedManager::SetSeed(12345);
    RngSeedManager::SetRun(seed);

    
    uint32_t maxConn  = 2;         
    double   areaX    = 1000.0;    
    double   areaY    = 1000.0;


    uint32_t gridWidth = (uint32_t)std::ceil(std::sqrt((double)nNodes));
    uint32_t gridRows  = (uint32_t)std::ceil((double)nNodes / gridWidth);
    double   gridDX    = areaX / (gridWidth + 1);
    double   gridDY    = areaY / (gridRows + 1);

    std::cout << " Nodes:     " << nNodes << "\n";
    std::cout << " Attack:    " << attack << "\n";
    std::cout << " Malicious: " << nMalicious << "\n";
    std::cout << " Defense:   " << defense << "\n";
    std::cout << " Speed:     " << speed << " m/s\n";
    std::cout << " SimTime:   " << simTime << " s\n";
    std::cout << " Grid:      " << gridWidth << "x" << gridRows
              << " (DX=" << gridDX << "m DY=" << gridDY << "m)\n";
    std::cout << " Area:      " << areaX << "x" << areaY << "m\n";
    std::cout << " Conns:     " << maxConn << "\n";
    if (attack == "grayhole" || attack == "smartgray")
        std::cout << " DropProb:  " << dropProb << "\n";
    if (defense == "mbdp")
        std::cout << " MBDP K:    " << mbdpK << "\n";
    std::cout << "========================================\n";

    // 1. Nodes
    NodeContainer nodes;
    nodes.Create(nNodes);

 
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode", StringValue("DsssRate11Mbps"),
                                 "ControlMode", StringValue("DsssRate1Mbps"));

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
   
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                    "MaxRange", DoubleValue(250.0));

    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

  
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(gridDX),
                                  "MinY", DoubleValue(gridDY),
                                  "DeltaX", DoubleValue(gridDX),
                                  "DeltaY", DoubleValue(gridDY),
                                  "GridWidth", UintegerValue(gridWidth),
                                  "LayoutType", StringValue("RowFirst"));

    if (speed == 0.0) {
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    } else {
        Ptr<RandomRectanglePositionAllocator> waypointAlloc =
            CreateObject<RandomRectanglePositionAllocator>();
        waypointAlloc->SetAttribute("X", StringValue("ns3::UniformRandomVariable[Min=0|Max=" + std::to_string(areaX) + "]"));
        waypointAlloc->SetAttribute("Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=" + std::to_string(areaY) + "]"));
        mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
            "Speed", StringValue("ns3::UniformRandomVariable[Min=1|Max=" + std::to_string(speed) + "]"),
            "Pause", StringValue("ns3::ConstantRandomVariable[Constant=2.0]"),
            "PositionAllocator", PointerValue(waypointAlloc));
    }
    mobility.Install(nodes);

   
    AodvHelper aodv;
    InternetStackHelper stack;
    stack.SetRoutingHelper(aodv);
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    
    std::set<uint32_t> maliciousIndices;
    if (attack != "none" && nMalicious > 0)
    {
        for (uint32_t i = 0; i < nMalicious && i < nNodes - 2; i++)
        {
            uint32_t idx = (nNodes / (nMalicious + 1)) * (i + 1);
            if (idx < maxConn) idx = maxConn;
            if (idx >= nNodes - maxConn) idx = nNodes - maxConn - 1;
            maliciousIndices.insert(idx);
        }
    }

  
    for (uint32_t i = 0; i < nNodes; i++)
    {
        Ptr<Ipv4> ipv4 = nodes.Get(i)->GetObject<Ipv4>();
        Ptr<aodv::RoutingProtocol> aodvProto =
            DynamicCast<aodv::RoutingProtocol>(ipv4->GetRoutingProtocol());

        if (!aodvProto) continue;

        bool isMalicious = (maliciousIndices.find(i) != maliciousIndices.end());

        if (isMalicious)
        {
            if (attack == "blackhole") {
                aodvProto->SetIsMalicious(true);
                aodvProto->SetMaliciousSeqNoBoost(1000);
                NS_LOG_UNCOND("** Node " << i << " -> BLACKHOLE **");
            }
            else if (attack == "smartblack") {
                aodvProto->SetIsSmartBlackhole(true);
                NS_LOG_UNCOND("** Node " << i << " -> SMART BLACKHOLE **");
            }
            else if (attack == "grayhole") {
                aodvProto->SetIsGrayhole(true);
                aodvProto->SetGrayholeDropProbability(dropProb);
                aodvProto->SetMaliciousSeqNoBoost(1000);
                NS_LOG_UNCOND("** Node " << i << " -> GRAYHOLE (" << (dropProb*100) << "%) **");
            }
            else if (attack == "smartgray") {
                aodvProto->SetIsSmartGrayhole(true);
                aodvProto->SetGrayholeDropProbability(dropProb);
                NS_LOG_UNCOND("** Node " << i << " -> SMART GRAYHOLE (" << (dropProb*100) << "%) **");
            }
        }
        else
        {
            if (defense == "ids") {
                aodvProto->SetEnableIDS(true);
            }
            else if (defense == "mbdp") {
                aodvProto->SetEnableMBDP(true);
                aodvProto->SetMbdpK(mbdpK);
            }
        }
    }

    if (defense == "ids") {
        NS_LOG_UNCOND("** Defense: IDS-AODV (ignore first RREP) **");
    } else if (defense == "mbdp") {
        NS_LOG_UNCOND("** Defense: MBDP-AODV (mean+stddev, K=" << mbdpK << ") **");
    }

    uint16_t port = 9;
    for (uint32_t i = 0; i < maxConn; i++) {
        uint32_t srcId = i;
        uint32_t dstId = nNodes - 1 - i;
        if (srcId >= dstId) break;

        PacketSinkHelper sink("ns3::UdpSocketFactory",
            InetSocketAddress(Ipv4Address::GetAny(), port + i));
        ApplicationContainer sinkApp = sink.Install(nodes.Get(dstId));
        sinkApp.Start(Seconds(0.0));
        sinkApp.Stop(Seconds(simTime));

        OnOffHelper onoff("ns3::UdpSocketFactory",
            InetSocketAddress(interfaces.GetAddress(dstId), port + i));
        onoff.SetAttribute("PacketSize", UintegerValue(512));
        onoff.SetConstantRate(DataRate("10kbps"));   // Paper: 10 kb CBR
        onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

        ApplicationContainer srcApp = onoff.Install(nodes.Get(srcId));
        srcApp.Start(Seconds(20.0));
        srcApp.Stop(Seconds(simTime - 3.0));
    }

    // 8. Flow Monitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    // 9. Calculate Results
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    double totalTx = 0, totalRx = 0, totalRxBytes = 0;
    uint64_t routingOverhead = 0;

    for (auto& flow : stats) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flow.first);

        if (t.destinationPort == 654 || t.sourcePort == 654) {
            routingOverhead += flow.second.txPackets;
            continue;
        }

        if (t.destinationPort < 9) continue;

        totalTx      += flow.second.txPackets;
        totalRx      += flow.second.rxPackets;
        totalRxBytes += flow.second.rxBytes;
    }

    double pdr = (totalTx > 0) ? (totalRx / totalTx) * 100.0 : 0.0;
    double throughput = (totalRxBytes * 8.0) / (simTime * 1000.0);
    double nrl = (totalRx > 0) ? (double)routingOverhead / totalRx : 0.0;

    NS_LOG_UNCOND("=== RESULTS ===");
    NS_LOG_UNCOND("PDR              = " << pdr << " %");
    NS_LOG_UNCOND("Throughput       = " << throughput << " Kbps");
    NS_LOG_UNCOND("Routing Overhead = " << routingOverhead);
    NS_LOG_UNCOND("NRL              = " << nrl);
    NS_LOG_UNCOND("(DataTx=" << totalTx << " DataRx=" << totalRx << " CtrlPkts=" << routingOverhead << ")");

    // CSV: nNodes,attack,nMalicious,defense,speed,pdr,throughput,routingOverhead,nrl
    std::cout << "CSV:" << nNodes << "," << attack << "," << nMalicious << ","
              << defense << "," << speed << ","
              << pdr << "," << throughput << ","
              << routingOverhead << "," << nrl << "\n";

    Simulator::Destroy();
    return 0;
}