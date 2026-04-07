\
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/yans-wifi-helper.h"

#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AodvBlackHoleTest");

class AodvBlackHoleExperiment
{
  public:
    AodvBlackHoleExperiment();
    void Run(int nNodes,
             double duration,
             bool enableBlackHole,
             int maliciousNodeId,
             std::string outputPrefix);

  private:
    void CreateNodes(int nNodes);
    void CreateDevices();
    void InstallInternetStack(bool enableBlackHole, int maliciousNodeId);
    void InstallApplications(int nConnections);
    void SetupMobility(double step);
    
    NodeContainer nodes;
    NetDeviceContainer devices;
    Ipv4InterfaceContainer interfaces;
    
    double m_totalTime;
    int m_nNodes;
};

AodvBlackHoleExperiment::AodvBlackHoleExperiment()
    : m_totalTime(40.0),
      m_nNodes(10)
{
}

void
AodvBlackHoleExperiment::CreateNodes(int nNodes)
{
    m_nNodes = nNodes;
    nodes.Create(nNodes);
}

void
AodvBlackHoleExperiment::CreateDevices()
{
  
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");
    
    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiPhy.SetChannel(wifiChannel.Create());
    
    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode", StringValue("OfdmRate6Mbps"),
                                 "RtsCtsThreshold", UintegerValue(0));
    
    devices = wifi.Install(wifiPhy, wifiMac, nodes);
}

void
AodvBlackHoleExperiment::SetupMobility(double step)
{
    MobilityHelper mobility;
    
    
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(step),
                                  "DeltaY", DoubleValue(0.0),  // 1D linear grid
                                  "GridWidth", UintegerValue(m_nNodes),
                                  "LayoutType", StringValue("RowFirst"));
    
   
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);
}

void
AodvBlackHoleExperiment::InstallInternetStack(bool enableBlackHole, int maliciousNodeId)
{
    AodvHelper aodv;
    InternetStackHelper internet;
    
    
    internet.SetRoutingHelper(aodv);
    internet.Install(nodes);
 
    if (enableBlackHole && maliciousNodeId >= 0 && maliciousNodeId < m_nNodes)
    {
        Ptr<Node> maliciousNode = nodes.Get(maliciousNodeId);
        Ptr<Ipv4> ipv4 = maliciousNode->GetObject<Ipv4>();
        Ptr<Ipv4RoutingProtocol> routing = ipv4->GetRoutingProtocol();
        Ptr<aodv::RoutingProtocol> aodvRouting = DynamicCast<aodv::RoutingProtocol>(routing);
        
        if (aodvRouting)
        {
            aodvRouting->SetIsMalicious(true);
            aodvRouting->SetMaliciousSeqNoBoost(1000);
            NS_LOG_INFO("Node " << maliciousNodeId << " configured as MALICIOUS (Black Hole attacker)");
        }
    }
    
 
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    interfaces = address.Assign(devices);
}

void
AodvBlackHoleExperiment::InstallApplications(int nConnections)
{
    
    uint16_t port = 9;
    
  
    int maxConnections = std::min(nConnections, m_nNodes / 2);
    
    for (int i = 0; i < maxConnections; i++)
    {
       
        int sourceId = i;
        // Destination node (opposite end)
        int destId = m_nNodes - 1 - i;
        
        if (sourceId >= m_nNodes || destId < 0 || sourceId == destId)
            continue;
        
        
        PacketSinkHelper sinkHelper("ns3::UdpSocketFactory",
                                    InetSocketAddress(Ipv4Address::GetAny(), port + i));
        ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(destId));
        sinkApp.Start(Seconds(0.0));
        sinkApp.Stop(Seconds(m_totalTime));
        
       
        OnOffHelper onOffHelper("ns3::UdpSocketFactory",
                                InetSocketAddress(interfaces.GetAddress(destId), port + i));
        onOffHelper.SetAttribute("PacketSize", UintegerValue(512));
        onOffHelper.SetAttribute("DataRate", StringValue("8192bps")); // ~16 packets/sec
        onOffHelper.SetConstantRate(DataRate("8192bps"));
        
        ApplicationContainer sourceApp = onOffHelper.Install(nodes.Get(sourceId));
        sourceApp.Start(Seconds(5.0));
        sourceApp.Stop(Seconds(m_totalTime - 1.0));
        
        NS_LOG_INFO("CBR Connection " << i << ": Node " << sourceId << " -> Node " << destId);
    }
}

void
AodvBlackHoleExperiment::Run(int nNodes,
                              double duration,
                              bool enableBlackHole,
                              int maliciousNodeId,
                              std::string outputPrefix)
{
    m_totalTime = duration;
    
    NS_LOG_INFO("Starting AODV Black Hole Attack Simulation");
    NS_LOG_INFO("  Nodes: " << nNodes);
    NS_LOG_INFO("  Duration: " << duration << " seconds");
    NS_LOG_INFO("  Black Hole Attack: " << (enableBlackHole ? "ENABLED" : "DISABLED"));
    if (enableBlackHole)
    {
        NS_LOG_INFO("  Malicious Node ID: " << maliciousNodeId);
    }
    
    
    CreateNodes(nNodes);
    CreateDevices();
    SetupMobility(50.0);  
    InstallInternetStack(enableBlackHole, maliciousNodeId);
    InstallApplications(4);  
    
    
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();
    
    
    NS_LOG_INFO("Running simulation...");
    Simulator::Stop(Seconds(m_totalTime));
    Simulator::Run();
    

    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    
    double totalTxPackets = 0;
    double totalRxPackets = 0;
    double totalTxBytes = 0;
    double totalRxBytes = 0;
    double totalDelay = 0;
    double totalRxPacketsForDelay = 0;
    
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin();
         i != stats.end();
         ++i)
    {
        totalTxPackets += i->second.txPackets;
        totalRxPackets += i->second.rxPackets;
        totalTxBytes += i->second.txBytes;
        totalRxBytes += i->second.rxBytes;
        
        if (i->second.rxPackets > 0)
        {
            totalDelay += i->second.delaySum.GetSeconds();
            totalRxPacketsForDelay += i->second.rxPackets;
        }
    }
    
  
    double pdr = (totalTxPackets > 0) ? (totalRxPackets / totalTxPackets) * 100.0 : 0.0;
    double throughput = (totalRxBytes * 8.0) / (m_totalTime * 1000.0);  // kbps
    double avgDelay = (totalRxPacketsForDelay > 0) ? (totalDelay / totalRxPacketsForDelay) * 1000.0
                                                    : 0.0;  // ms
    


    std::cout << "Configuration:\n";
    std::cout << "  Nodes: " << nNodes << "\n";
    std::cout << "  Duration: " << duration << " s\n";
    std::cout << "  Black Hole Attack: " << (enableBlackHole ? "ENABLED" : "DISABLED") << "\n";
    if (enableBlackHole)
    {
        std::cout << "  Malicious Node: " << maliciousNodeId << "\n";
    }
    std::cout << "\nPerformance Metrics:\n";
    std::cout << "  Packets Sent: " << totalTxPackets << "\n";
    std::cout << "  Packets Received: " << totalRxPackets << "\n";
    std::cout << "  Packet Delivery Ratio (PDR): " << pdr << " %\n";
    std::cout << "  Average Throughput: " << throughput << " kbps\n";
    std::cout << "  Average End-to-End Delay: " << avgDelay << " ms\n";
    std::cout << "========================================\n\n";
    
   
    std::string filename = outputPrefix + "_results.txt";
    std::ofstream outFile(filename, std::ios::app);
    outFile << nNodes << "," << (enableBlackHole ? "1" : "0") << "," << pdr << "," << throughput
            << "," << avgDelay << "," << totalTxPackets << "," << totalRxPackets << "\n";
    outFile.close();
    
   
    monitor->SerializeToXmlFile(outputPrefix + "_flowmon.xml", true, true);
    
    Simulator::Destroy();
    NS_LOG_INFO("Simulation completed successfully");
}

int
main(int argc, char* argv[])
{
    // Default parameters
    int nNodes = 10;
    double duration = 40.0;
    bool enableBlackHole = true;
    int maliciousNodeId = 10;
    std::string outputPrefix = "blackhole_test";
    
   
    CommandLine cmd;
    cmd.AddValue("nodes", "Number of nodes", nNodes);
    cmd.AddValue("duration", "Simulation duration (seconds)", duration);
    cmd.AddValue("enableBlackHole", "Enable Black Hole attack (true/false)", enableBlackHole);
    cmd.AddValue("maliciousNode", "ID of malicious node", maliciousNodeId);
    cmd.AddValue("output", "Output file prefix", outputPrefix);
    cmd.Parse(argc, argv);
    
    
    LogComponentEnable("AodvBlackHoleTest", LOG_LEVEL_INFO);
    

    AodvBlackHoleExperiment experiment;
    experiment.Run(nNodes, duration, enableBlackHole, maliciousNodeId, outputPrefix);
    
    return 0;
}
