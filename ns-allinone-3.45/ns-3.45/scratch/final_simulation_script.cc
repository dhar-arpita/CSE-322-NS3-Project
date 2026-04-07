

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
#include <string>
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AodvAttackSim");

int main(int argc, char *argv[])
{
  
    uint32_t    nNodes     = 50;
    uint32_t    nFlows     = 2;       
    uint32_t    pps        = 0;       
    uint32_t    areaMult   = 0;      
    std::string scenario   = "";      
    std::string attack     = "none";
    std::string defense    = "none";
    uint32_t    nMalicious = 1;
    double      dropProb   = 0.5;
    double      speed      = 0.0;
    double      simTime    = 100.0;
    uint32_t    seed       = 30;
    uint32_t    mbdpK      = 4;
    double      trustThreshold = 0.6;
    uint32_t    packetSize = 512;
    std::string outputFile = "";      

    CommandLine cmd;
    cmd.AddValue("nNodes",     "Number of nodes",                              nNodes);
    cmd.AddValue("nFlows",     "Number of flows (default 2)",                  nFlows);
    cmd.AddValue("pps",        "Total packets/sec (0=paper default 10kbps)",   pps);
    cmd.AddValue("areaMult",   "Area = areaMult * 250m (0=paper 750x750)",     areaMult);
    cmd.AddValue("scenario",   "Scenario: normal/grayhole_none/grayhole_trust/smartgray_none/smartgray_trust", scenario);
    cmd.AddValue("attack",     "Attack: none/blackhole/smartblack/grayhole/smartgray", attack);
    cmd.AddValue("defense",    "Defense: none/ids/mbdp/trust",                 defense);
    cmd.AddValue("nMalicious", "Number of malicious nodes",                    nMalicious);
    cmd.AddValue("dropProb",   "Grayhole drop probability (0.0-1.0)",          dropProb);
    cmd.AddValue("speed",      "Max node speed m/s (0=static)",                speed);
    cmd.AddValue("simTime",    "Simulation time seconds",                      simTime);
    cmd.AddValue("seed",       "RNG seed",                                     seed);
    cmd.AddValue("mbdpK",      "MBDP-AODV: replies to collect",               mbdpK);
    cmd.AddValue("trustThreshold", "Trust threshold",                          trustThreshold);
    cmd.AddValue("outputFile", "CSV output file path",                         outputFile);
    cmd.Parse(argc, argv);

    RngSeedManager::SetSeed(12345);
    RngSeedManager::SetRun(seed);

 
    if (!scenario.empty())
    {
        if (scenario == "normal") {
            attack = "none"; defense = "none";
        } else if (scenario == "grayhole_none") {
            attack = "grayhole"; defense = "none";
        } else if (scenario == "grayhole_ids") {
            attack = "grayhole"; defense = "ids";
        } else if (scenario == "grayhole_mbdp") {
            attack = "grayhole"; defense = "mbdp";
        } else if (scenario == "grayhole_trust") {
            attack = "grayhole"; defense = "trust";
        } else if (scenario == "smartgray_none") {
            attack = "smartgray"; defense = "none";
        } else if (scenario == "smartgray_ids") {
            attack = "smartgray"; defense = "ids";
        } else if (scenario == "smartgray_mbdp") {
            attack = "smartgray"; defense = "mbdp";
        } else if (scenario == "smartgray_trust") {
            attack = "smartgray"; defense = "trust";
        }
    }
    else
    {
        
        scenario = attack + "_" + defense;
    }

  
    double txRange = 250.0;
    double areaX, areaY;
    if (areaMult > 0) {
        areaX = areaMult * txRange;
        areaY = areaMult * txRange;
    } else {
        areaX = 750.0;  
        areaY = 750.0;
    }

    uint32_t gridWidth = (uint32_t)std::ceil(std::sqrt((double)nNodes));
    uint32_t gridRows  = (uint32_t)std::ceil((double)nNodes / gridWidth);
    double   gridDX = areaX / (gridWidth + 1);
    double   gridDY = areaY / (gridRows + 1);

   
    uint32_t actualFlows = std::min(nFlows, nNodes / 2);
    std::string dataRateStr;
    if (pps > 0) {
        
        uint32_t ppsPerFlow = std::max((uint32_t)1, pps / std::max((uint32_t)1, actualFlows));
        uint64_t bps = (uint64_t)ppsPerFlow * packetSize * 8;
        std::ostringstream oss;
        oss << bps << "bps";
        dataRateStr = oss.str();
    } else {
      
        dataRateStr = "10240bps";
    }


    std::cout << " Nodes:     " << nNodes << "\n";
    std::cout << " Flows:     " << actualFlows << "\n";
    std::cout << " DataRate:  " << dataRateStr << " per flow\n";
    std::cout << " Area:      " << areaX << "x" << areaY << "m\n";
    std::cout << " Attack:    " << attack << "\n";
    std::cout << " Defense:   " << defense << "\n";
    std::cout << " Scenario:  " << scenario << "\n";
    std::cout << " Malicious: " << nMalicious << "\n";
    std::cout << " Speed:     " << speed << " m/s\n";
    std::cout << " Grid:      " << gridWidth << "x" << gridRows
              << " (DX=" << gridDX << "m DY=" << gridDY << "m)\n";
    if (attack == "grayhole" || attack == "smartgray")
        std::cout << " DropProb:  " << dropProb << "\n";
    if (defense == "mbdp")
        std::cout << " MBDP K:    " << mbdpK << "\n";
    if (defense == "trust")
        std::cout << " Trust Thr: " << trustThreshold << "\n";

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
                                    "MaxRange", DoubleValue(txRange));

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
            if (idx < actualFlows) idx = actualFlows;
            if (idx >= nNodes - actualFlows) idx = nNodes - actualFlows - 1;
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
            else if (defense == "trust") {
                aodvProto->SetEnableTrust(true);
                aodvProto->SetTrustThreshold(trustThreshold);
            }
        }
    }

    if (defense == "ids") {
        NS_LOG_UNCOND("** Defense: IDS-AODV (ignore first RREP) **");
    } else if (defense == "mbdp") {
        NS_LOG_UNCOND("** Defense: MBDP-AODV (mean+stddev, K=" << mbdpK << ") **");
    } else if (defense == "trust") {
        NS_LOG_UNCOND("** Defense: Trust-Based AODV (threshold=" << trustThreshold << ") **");
    }

   
    uint16_t basePort = 9;
    for (uint32_t i = 0; i < actualFlows; i++)
    {
        uint32_t srcId = i;
        uint32_t dstId = nNodes - 1 - i;
        if (srcId >= dstId) break;

        uint16_t port = basePort + i;

        PacketSinkHelper sink("ns3::UdpSocketFactory",
            InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer sinkApp = sink.Install(nodes.Get(dstId));
        sinkApp.Start(Seconds(0.0));
        sinkApp.Stop(Seconds(simTime));

        OnOffHelper onoff("ns3::UdpSocketFactory",
            InetSocketAddress(interfaces.GetAddress(dstId), port));
        onoff.SetAttribute("PacketSize", UintegerValue(packetSize));
        onoff.SetConstantRate(DataRate(dataRateStr));
        onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

        ApplicationContainer srcApp = onoff.Install(nodes.Get(srcId));
        srcApp.Start(Seconds(20.0));
        srcApp.Stop(Seconds(simTime - 3.0));
    }


    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

  
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    double totalTx = 0, totalRx = 0, totalRxBytes = 0;
    double totalDelay = 0;
    uint64_t routingOverhead = 0;

    for (auto& flow : stats)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flow.first);

        if (t.destinationPort == 654 || t.sourcePort == 654) {
            routingOverhead += flow.second.txPackets;
            continue;
        }

        if (t.destinationPort < basePort) continue;

        totalTx      += flow.second.txPackets;
        totalRx      += flow.second.rxPackets;
        totalRxBytes += flow.second.rxBytes;

        if (flow.second.rxPackets > 0) {
            totalDelay += flow.second.delaySum.GetSeconds();
        }
    }

    double pdr = (totalTx > 0) ? (totalRx / totalTx) * 100.0 : 0.0;
    double dropRatio = (totalTx > 0) ? ((totalTx - totalRx) / totalTx) * 100.0 : 0.0;
    double throughput = (totalRxBytes * 8.0) / (simTime * 1000.0);  
    double avgDelay = (totalRx > 0) ? (totalDelay / totalRx) * 1000.0 : 0.0;
    double nrl = (totalRx > 0) ? (double)routingOverhead / totalRx : 0.0;

    
    double txPowerW = 1.65;
    double rxPowerW = 1.4;
    double idlePowerW = 1.15;
    double packetTimeS = (double)(packetSize * 8) / 11000000.0;
    double txEnergy = totalTx * packetTimeS * txPowerW;
    double rxEnergy = totalRx * packetTimeS * rxPowerW;
    double ctrlEnergy = routingOverhead * packetTimeS * (txPowerW + rxPowerW) / 2.0;
    double idleEnergy = nNodes * simTime * idlePowerW * 0.001;
    double totalEnergy = txEnergy + rxEnergy + ctrlEnergy + idleEnergy;

    NS_LOG_UNCOND("=== RESULTS ===");
    NS_LOG_UNCOND("PDR              = " << pdr << " %");
    NS_LOG_UNCOND("Drop Ratio       = " << dropRatio << " %");
    NS_LOG_UNCOND("Throughput       = " << throughput << " Kbps");
    NS_LOG_UNCOND("Avg Delay        = " << avgDelay << " ms");
    NS_LOG_UNCOND("Energy           = " << totalEnergy << " J");
    NS_LOG_UNCOND("Routing Overhead = " << routingOverhead);
    NS_LOG_UNCOND("NRL              = " << nrl);
    NS_LOG_UNCOND("(DataTx=" << totalTx << " DataRx=" << totalRx << " CtrlPkts=" << routingOverhead << ")");


    uint32_t areaMultOut = (areaMult > 0) ? areaMult : 3;  // paper default = 750/250 = 3
    uint32_t ppsOut = pps;
    if (pps == 0) {
       
        ppsOut = (uint32_t)(actualFlows * 10240.0 / (packetSize * 8));
    }

    std::string csvLine = std::to_string(nNodes) + ","
                        + std::to_string(nFlows) + ","
                        + std::to_string(ppsOut) + ","
                        + std::to_string(areaMultOut) + ","
                        + scenario + ","
                        + std::to_string(throughput) + ","
                        + std::to_string(avgDelay) + ","
                        + std::to_string(pdr) + ","
                        + std::to_string(dropRatio) + ","
                        + std::to_string(totalEnergy);

    std::cout << "CSV:" << csvLine << "\n";

    if (!outputFile.empty()) {
        std::ofstream ofs(outputFile, std::ios::app);
        ofs << csvLine << "\n";
        ofs.close();
    }

    Simulator::Destroy();
    return 0;
}