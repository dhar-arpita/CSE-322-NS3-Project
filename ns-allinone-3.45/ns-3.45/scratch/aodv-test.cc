
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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AodvBlackHoleTest");

int main(int argc, char *argv[])
{
    
    uint32_t    nNodes     = 20;
    uint32_t    nBlackhole = 0;
    double      speed      = 0.0;
    double      simTime    = 200.0;
    uint32_t    nConn      = 10;
    uint32_t    seed       = 30;
    std::string output     = "kolade";

    CommandLine cmd;
    cmd.AddValue("nNodes",     "Number of nodes (20,40,60,80,100)", nNodes);
    cmd.AddValue("nBlackhole", "Number of blackhole nodes (0-3)",   nBlackhole);
    cmd.AddValue("speed",      "Max node speed m/s (0-30)",         speed);
    cmd.AddValue("simTime",    "Simulation time seconds",           simTime);
    cmd.AddValue("nConn",      "Number of CBR connections",         nConn);
    cmd.AddValue("seed",       "RNG seed for reproducibility",      seed);
    cmd.AddValue("output",     "Output file prefix",                output);
    cmd.Parse(argc, argv);

    if (nBlackhole >= nNodes) {
        std::cerr << "Error: nBlackhole must be less than nNodes\n";
        return 1;
    }

    RngSeedManager::SetSeed(12345);
    RngSeedManager::SetRun(seed);

    uint32_t maxConn = std::min(nConn, nNodes / 4);
    if (maxConn < 2) maxConn = 2;


    std::cout << " Nodes       : " << nNodes     << "\n";
    std::cout << " Blackhole   : " << nBlackhole << "\n";
    std::cout << " Speed       : " << speed      << " m/s\n";
    std::cout << " SimTime     : " << simTime    << " s\n";
    std::cout << " Connections : " << maxConn    << "\n";
    std::cout << " Seed        : " << seed       << "\n";
    std::cout << " Area        : 1000x1500 (scaled by density)\n";
    std::cout << " WiFi Range  : 250 m\n";
  


    NodeContainer nodes;
    nodes.Create(nNodes);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",    StringValue("DsssRate11Mbps"),
                                 "ControlMode", StringValue("DsssRate1Mbps"));

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                    "MaxRange", DoubleValue(350.0));

    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);


    uint32_t gridWidth = (uint32_t)std::ceil(std::sqrt((double)nNodes));
    double   scaleFactor = std::sqrt((double)nNodes / 100.0);
    double   areaX     = 1000.0 * scaleFactor;
    double   areaY     = 1500.0 * scaleFactor;
    double   gridDX    = 100.0;   // 100m < 250m range = neighbors connected
    double   gridDY    = 100.0;

    NS_LOG_UNCOND("Area (scaled): " << (int)areaX << " x " << (int)areaY << " m");

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",       DoubleValue(50.0),
                                  "MinY",       DoubleValue(50.0),
                                  "DeltaX",     DoubleValue(gridDX),
                                  "DeltaY",     DoubleValue(gridDY),
                                  "GridWidth",  UintegerValue(gridWidth),
                                  "LayoutType", StringValue("RowFirst"));

    if (speed == 0.0) {
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    } else {
        Ptr<RandomRectanglePositionAllocator> waypointAlloc =
            CreateObject<RandomRectanglePositionAllocator>();
        waypointAlloc->SetAttribute(
            "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=" +
                              std::to_string(areaX) + "]"));
        waypointAlloc->SetAttribute(
            "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=" +
                              std::to_string(areaY) + "]"));

        mobility.SetMobilityModel(
            "ns3::RandomWaypointMobilityModel",
            "Speed", StringValue("ns3::UniformRandomVariable[Min=1|Max=" +
                                  std::to_string(speed) + "]"),
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

 
    if (nBlackhole > 0) {
        uint32_t startIdx = maxConn;                    
        uint32_t endIdx   = nNodes - maxConn;           
        uint32_t range    = endIdx - startIdx;

        for (uint32_t i = 0; i < nBlackhole; i++) {
            uint32_t bhIdx;
            if (nBlackhole == 1) {
                bhIdx = nNodes / 2;
            } else {
                bhIdx = startIdx + (uint32_t)((double)range * (i + 0.5) / nBlackhole);
            }
            if (bhIdx >= endIdx) bhIdx = endIdx - 1;

            Ptr<Ipv4> ipv4 = nodes.Get(bhIdx)->GetObject<Ipv4>();
            Ptr<aodv::RoutingProtocol> aodvProto =
                DynamicCast<aodv::RoutingProtocol>(ipv4->GetRoutingProtocol());
            if (aodvProto) {
                aodvProto->SetIsMalicious(true);
                aodvProto->SetMaliciousSeqNoBoost(1000);
                NS_LOG_UNCOND("** Node " << bhIdx << " -> BLACKHOLE **");
            }
        }
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
        onoff.SetConstantRate(DataRate("8192bps"));
        onoff.SetAttribute("OnTime",
            StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        onoff.SetAttribute("OffTime",
            StringValue("ns3::ConstantRandomVariable[Constant=0]"));

        ApplicationContainer srcApp = onoff.Install(nodes.Get(srcId));
        srcApp.Start(Seconds(20.0));    // let AODV routes converge
        srcApp.Stop(Seconds(simTime - 3.0));

        NS_LOG_UNCOND("CBR: Node " << srcId << " -> Node " << dstId);
    }

  
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    NS_LOG_UNCOND("Simulation running...");
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();


    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    double totalTx         = 0;
    double totalRx         = 0;
    double totalRxBytes    = 0;
    double totalDelay      = 0;
    double totalRxForDelay = 0;

    for (auto& flow : stats) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flow.first);
        
        if (t.destinationPort == 654) continue;
        if (t.destinationPort < 9) continue;

        totalTx      += flow.second.txPackets;
        totalRx      += flow.second.rxPackets;
        totalRxBytes += flow.second.rxBytes;

        if (flow.second.rxPackets > 0) {
            totalDelay      += flow.second.delaySum.GetSeconds();
            totalRxForDelay += flow.second.rxPackets;
        }
    }

    double pdr        = (totalTx > 0) ? (totalRx / totalTx) * 100.0 : 0.0;
    double throughput = (totalRxBytes * 8.0) / (simTime * 1000.0);
    double avgDelay   = (totalRxForDelay > 0) ?
                        (totalDelay / totalRxForDelay) * 1000.0 : 0.0;


    NS_LOG_UNCOND("Nodes          : " << nNodes);
    NS_LOG_UNCOND("Blackhole Nodes: " << nBlackhole);
    NS_LOG_UNCOND("Speed          : " << speed << " m/s");
    NS_LOG_UNCOND("Connections    : " << maxConn);
    NS_LOG_UNCOND("Packets Sent   : " << totalTx);
    NS_LOG_UNCOND("Packets Recv   : " << totalRx);
    NS_LOG_UNCOND("PDR            : " << pdr        << " %");
    NS_LOG_UNCOND("Throughput     : " << throughput << " Kbps");
    NS_LOG_UNCOND("Avg E2E Delay  : " << avgDelay   << " ms");
 

    std::cout << "CSV:" << nNodes     << ","
                        << nBlackhole << ","
                        << speed      << ","
                        << pdr        << ","
                        << throughput << ","
                        << avgDelay   << "\n";

    std::ofstream csv(output + "_results.csv", std::ios::app);
    csv << nNodes     << ","
        << nBlackhole << ","
        << speed      << ","
        << pdr        << ","
        << throughput << ","
        << avgDelay   << "\n";
    csv.close();

    Simulator::Destroy();
    return 0;
}