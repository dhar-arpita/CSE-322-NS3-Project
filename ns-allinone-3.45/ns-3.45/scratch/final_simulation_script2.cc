
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/aodv-module.h"
#include "ns3/arp-cache.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"

#include <fstream>
#include <iostream>
#include <cmath>
#include <set>
#include <string>
#include <sstream>

using namespace ns3;
using namespace ns3::lrwpan;

NS_LOG_COMPONENT_DEFINE("FinalSim");


void PopulateArpCache(NodeContainer& nodes,
                      Ipv4InterfaceContainer& interfaces,
                      NetDeviceContainer& devices)
{
    for (uint32_t i = 0; i < nodes.GetN(); i++)
    {
        Ptr<Ipv4L3Protocol> ipv4L3 = nodes.Get(i)->GetObject<Ipv4L3Protocol>();
   
        Ptr<Ipv4Interface> iface = ipv4L3->GetInterface(1);
        Ptr<ArpCache> arpCache = iface->GetArpCache();
        if (!arpCache)
        {
            arpCache = CreateObject<ArpCache>();
            arpCache->SetDevice(devices.Get(i), iface);
            iface->SetArpCache(arpCache);
        }
        arpCache->SetAliveTimeout(Seconds(3600)); // 1 hour — static network

       
        for (uint32_t j = 0; j < nodes.GetN(); j++)
        {
            if (i == j) continue;

            Ipv4Address ipAddr = interfaces.GetAddress(j);
            Address     macAddr = devices.Get(j)->GetAddress();

            ArpCache::Entry* entry = arpCache->Add(ipAddr);
            entry->SetMacAddress(macAddr);
            entry->MarkPermanent();
        }
    }
    NS_LOG_UNCOND("ARP cache populated for all " << nodes.GetN() << " nodes.");
}

int main(int argc, char *argv[])
{

    uint32_t    nNodes     = 40;
    uint32_t    nFlows     = 20;
    uint32_t    pps        = 200;
    uint32_t    areaMult   = 3;
    std::string scenario   = "normal";
    double      simTime    = 100.0;
    uint32_t    seed       = 30;
    uint32_t    packetSize = 80;
    double      txRange    = 50.0;
    double      dropProb   = 0.5;
    uint32_t    nMalicious = 1;
    std::string outputFile = "";

    std::string attack  = "none";
    std::string defense = "none";

    CommandLine cmd;
    cmd.AddValue("nNodes",     "Number of nodes",           nNodes);
    cmd.AddValue("nFlows",     "Number of flows",           nFlows);
    cmd.AddValue("pps",        "Total packets per second",  pps);
    cmd.AddValue("areaMult",   "Area multiplier",           areaMult);
    cmd.AddValue("scenario",   "Scenario string",           scenario);
    cmd.AddValue("simTime",    "Simulation time (s)",       simTime);
    cmd.AddValue("seed",       "RNG seed",                  seed);
    cmd.AddValue("dropProb",   "Grayhole drop probability", dropProb);
    cmd.AddValue("nMalicious", "Number of malicious nodes", nMalicious);
    cmd.AddValue("outputFile", "CSV output file path",      outputFile);
    cmd.Parse(argc, argv);

    RngSeedManager::SetSeed(12345);
    RngSeedManager::SetRun(seed);


    if      (scenario == "normal")           { attack = "none";      defense = "none";  }
    else if (scenario == "grayhole_none")    { attack = "grayhole";  defense = "none";  }
    else if (scenario == "grayhole_ids")     { attack = "grayhole";  defense = "ids";   }
    else if (scenario == "grayhole_mbdp")    { attack = "grayhole";  defense = "mbdp";  }
    else if (scenario == "grayhole_trust")   { attack = "grayhole";  defense = "trust"; }
    else if (scenario == "smartgray_none")   { attack = "smartgray"; defense = "none";  }
    else if (scenario == "smartgray_ids")    { attack = "smartgray"; defense = "ids";   }
    else if (scenario == "smartgray_mbdp")   { attack = "smartgray"; defense = "mbdp";  }
    else if (scenario == "smartgray_trust")  { attack = "smartgray"; defense = "trust"; }


    double   areaSize    = areaMult * txRange;
    uint32_t gridWidth   = (uint32_t)std::ceil(std::sqrt((double)nNodes));
    double   gridSpacing = areaSize / (gridWidth + 1);
    uint32_t actualFlows = std::min(nFlows, nNodes / 2);
    uint32_t ppsPerFlow  = std::max((uint32_t)1, pps / std::max((uint32_t)1, actualFlows));
    uint64_t dataRateBps = (uint64_t)ppsPerFlow * packetSize * 8;

    std::ostringstream dataRateStr;
    dataRateStr << dataRateBps << "bps";


    std::cout << " Network:   802.15.4 (LR-WPAN) + IPv4 + AODV\n";
    std::cout << " Nodes:     " << nNodes     << "\n";
    std::cout << " Flows:     " << actualFlows << "\n";
    std::cout << " PPS/flow:  " << ppsPerFlow  << "\n";
    std::cout << " DataRate:  " << dataRateStr.str() << " per flow\n";
    std::cout << " Area:      " << areaSize << "x" << areaSize << "m\n";
    std::cout << " Scenario:  " << scenario  << "\n";
    std::cout << " Attack:    " << attack     << "\n";
    std::cout << " Defense:   " << defense    << "\n";
    


    NodeContainer nodes;
    nodes.Create(nNodes);

   
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",       DoubleValue(gridSpacing),
                                  "MinY",       DoubleValue(gridSpacing),
                                  "DeltaX",     DoubleValue(gridSpacing),
                                  "DeltaY",     DoubleValue(gridSpacing),
                                  "GridWidth",  UintegerValue(gridWidth),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

  
    LrWpanHelper lrWpanHelper;
    lrWpanHelper.SetPropagationDelayModel("ns3::ConstantSpeedPropagationDelayModel");
    lrWpanHelper.AddPropagationLossModel("ns3::LogDistancePropagationLossModel");
    NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(nodes);

  
    lrWpanHelper.CreateAssociatedPan(lrwpanDevices, 0);

    
    AodvHelper aodv;
    InternetStackHelper stack;
    stack.SetRoutingHelper(aodv);
    stack.Install(nodes);

   
    Ipv4AddressHelper ipv4addr;
    ipv4addr.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4addr.Assign(lrwpanDevices);


    PopulateArpCache(nodes, interfaces, lrwpanDevices);


    std::set<uint32_t> maliciousNodes;

    if (attack != "none" && nMalicious > 0)
    {
        for (uint32_t i = 0; i < nMalicious && i < nNodes - 2; i++)
        {
            uint32_t idx = (nNodes / (nMalicious + 1)) * (i + 1);
            if (idx < actualFlows)             idx = actualFlows;
            if (idx >= nNodes - actualFlows)   idx = nNodes - actualFlows - 1;
            maliciousNodes.insert(idx);
            NS_LOG_UNCOND("** Malicious node: " << idx << " (" << attack << ") **");
        }
    }


    for (uint32_t nodeId : maliciousNodes)
    {
        Ptr<Ipv4> ipv4ptr = nodes.Get(nodeId)->GetObject<Ipv4>();
        Ptr<aodv::RoutingProtocol> aodvRp =
            DynamicCast<aodv::RoutingProtocol>(ipv4ptr->GetRoutingProtocol());

        if (!aodvRp)
        {
            NS_LOG_WARN("AODV not found on node " << nodeId);
            continue;
        }

        if (attack == "grayhole")
        {
            aodvRp->SetIsGrayhole(true);
            aodvRp->SetGrayholeDropProbability(dropProb);
        }
        else if (attack == "smartgray")
        {
            aodvRp->SetIsSmartGrayhole(true);
            aodvRp->SetGrayholeDropProbability(dropProb);
        }
    }


    for (uint32_t i = 0; i < nNodes; i++)
    {
        if (maliciousNodes.count(i)) continue;

        Ptr<Ipv4> ipv4ptr = nodes.Get(i)->GetObject<Ipv4>();
        Ptr<aodv::RoutingProtocol> aodvRp =
            DynamicCast<aodv::RoutingProtocol>(ipv4ptr->GetRoutingProtocol());
        if (!aodvRp) continue;

        if (defense == "trust") aodvRp->SetEnableTrust(true);
        if (defense == "ids")   aodvRp->SetEnableIDS(true);
    }

   
    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> flowMonitor = flowHelper.InstallAll();


    uint16_t basePort = 9;

    for (uint32_t i = 0; i < actualFlows; i++)
    {
        uint32_t srcId = i;
        uint32_t dstId = nNodes - 1 - i;
        if (srcId >= dstId) break;

        uint16_t port    = basePort + i;
        Ipv4Address dstAddr = interfaces.GetAddress(dstId);

        // Sink
        PacketSinkHelper sink("ns3::UdpSocketFactory",
                              InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer sinkApp = sink.Install(nodes.Get(dstId));
        sinkApp.Start(Seconds(0.0));
        sinkApp.Stop(Seconds(simTime));

        // Source
        OnOffHelper onoff("ns3::UdpSocketFactory",
                          InetSocketAddress(dstAddr, port));
        onoff.SetAttribute("PacketSize", UintegerValue(packetSize));
        onoff.SetConstantRate(DataRate(dataRateStr.str()));
        onoff.SetAttribute("OnTime",  StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

        ApplicationContainer srcApp = onoff.Install(nodes.Get(srcId));
        srcApp.Start(Seconds(10.0));
        srcApp.Stop(Seconds(simTime - 2.0));
    }

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

   
    flowMonitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats();

    double   totalTxPackets = 0;
    double   totalRxPackets = 0;
    double   totalRxBytes   = 0;
    double   totalDelay     = 0;
    uint32_t delayCount     = 0;

    for (auto& kv : stats)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(kv.first);
        if (t.protocol != 17) continue;  // UDP only
        if (t.destinationPort < basePort ||
            t.destinationPort >= basePort + actualFlows) continue;

        totalTxPackets += kv.second.txPackets;
        totalRxPackets += kv.second.rxPackets;
        totalRxBytes   += kv.second.rxBytes;

        if (kv.second.rxPackets > 0)
        {
            totalDelay += kv.second.delaySum.GetSeconds() * 1000.0;
            delayCount += kv.second.rxPackets;
        }
    }

   
    double pdr_val    = (totalTxPackets > 0)
                        ? (totalRxPackets / totalTxPackets) * 100.0 : 0.0;
    double dropRatio  = 100.0 - pdr_val;
    if (dropRatio < 0) dropRatio = 0;

    double throughput = (totalRxBytes * 8.0) / (simTime * 1000.0); // Kbps
    double avgDelay   = (delayCount > 0) ? (totalDelay / delayCount) : 0.0;


    double txPowerW   = 0.0522;
    double rxPowerW   = 0.0591;
    double idlePowerW = 0.00128;
    double pktTimeS   = (double)(packetSize * 8) / 250000.0;
    double txEnergy   = totalTxPackets * pktTimeS * txPowerW;
    double rxEnergy   = totalRxPackets * pktTimeS * rxPowerW;
    double idleEnergy = nNodes * simTime * idlePowerW;
    double totalEnergy = txEnergy + rxEnergy + idleEnergy;

  
    NS_LOG_UNCOND("=== RESULTS (802.15.4 + IPv4 + AODV) ===");
    NS_LOG_UNCOND("Scenario     = " << scenario);
    NS_LOG_UNCOND("PDR          = " << pdr_val    << " %");
    NS_LOG_UNCOND("Drop Ratio   = " << dropRatio  << " %");
    NS_LOG_UNCOND("Throughput   = " << throughput << " Kbps");
    NS_LOG_UNCOND("Avg Delay    = " << avgDelay   << " ms");
    NS_LOG_UNCOND("Energy       = " << totalEnergy << " J");
    NS_LOG_UNCOND("Tx=" << totalTxPackets << " Rx=" << totalRxPackets);


    std::string csvLine =
        std::to_string(nNodes)     + "," +
        std::to_string(nFlows)     + "," +
        std::to_string(pps)        + "," +
        std::to_string(areaMult)   + "," +
        scenario                   + "," +
        std::to_string(throughput) + "," +
        std::to_string(avgDelay)   + "," +
        std::to_string(pdr_val)    + "," +
        std::to_string(dropRatio)  + "," +
        std::to_string(totalEnergy);

    std::cout << "CSV:" << csvLine << "\n";

    if (!outputFile.empty())
    {
        std::ofstream ofs(outputFile, std::ios::app);
        ofs << csvLine << "\n";
        ofs.close();
    }

    Simulator::Destroy();
    return 0;
}