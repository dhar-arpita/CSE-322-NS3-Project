/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * ===================================================================
 *  second.cc — NS-3 er SECOND example script
 *  CSE 322: Computer Networks
 * ===================================================================
 *
 *  PURA CODE KI KORCHHE? (Big Picture)
 *  ------------------------------------
 *  Duita ALAG network banacche ar connect koreche:
 *
 *         10.1.1.0 (P2P)                  10.1.2.0 (CSMA LAN)
 *     n0 ──────────────── n1 ───┬─── n2
 *     (Client)  5Mbps,2ms  |    ├─── n3
 *                    (BRIDGE)   └─── n4 (Server)
 *                               CSMA Bus
 *                            100Mbps, 6560ns
 *
 *  LEFT SIDE:  n0 ↔ n1 → Point-to-Point (direct private cable)
 *  RIGHT SIDE: n1, n2, n3, n4 → CSMA LAN (shared bus, like Ethernet)
 *
 *  n1 = BRIDGE — duita network e eksathe connected!
 *  n1 er 2 ta NIC ache: ekta P2P (10.1.1.2), ekta CSMA (10.1.2.1)
 *
 *  Client = n0 (10.1.1.1) — P2P network e
 *  Server = n4 (10.1.2.4) — CSMA LAN er shesh node
 *
 *  Packet journey: n0 → P2P cable → n1 (bridge) → CSMA bus → n4
 *                  n4 echo kore → CSMA bus → n1 → P2P cable → n0
 *
 *  first.cc vs second.cc:
 *  ─────────────────────────────────────────────────
 *  first.cc:  1 network, 2 nodes, no routing needed
 *  second.cc: 2 networks, 5 nodes, ROUTING LAGBE!
 *  ─────────────────────────────────────────────────
 *
 *  NOTUN concepts in second.cc:
 *    1. CSMA (shared bus LAN)
 *    2. Routing (PopulateRoutingTables)
 *    3. CommandLine arguments (nCsma, verbose)
 *    4. PCAP packet capture
 *    5. n1 er dual role (bridge between 2 networks)
 * ===================================================================
 */


/* ===================================================================
 *  BLOCK 1: HEADER FILES (#include)
 * ===================================================================
 *  first.cc te 5 ta chilo, ekhane 7 ta — 2 ta NOTUN:
 *    csma-module.h              → CSMA LAN network banate lagbe
 *    ipv4-global-routing-helper → Routing table auto-populate korte lagbe
 */

// UdpEchoClient, UdpEchoServer — actual apps
#include "ns3/applications-module.h"

// NS-3 er engine — Simulator, Time, CommandLine, Logging
#include "ns3/core-module.h"

// ── NOTUN! ──
// CSMA = Carrier Sense Multiple Access
// Meeting room er shared microphone er moto — shobai EKTA bus share kore
// kotha (data) bolar age shune "keu boltechhe ki na?" — na bolле, tui bolo
// Multiple computer EKTA shared cable/bus e connected
#include "ns3/csma-module.h"

// TCP/IP stack, IP addressing
#include "ns3/internet-module.h"

// ── NOTUN! ──
// Routing table automatically populate kore
// first.cc te lageni karon same network e chilo (10.1.1.0)
// Ekhane 2 ta ALAG network (10.1.1.0 + 10.1.2.0) — routing MUST!
// Analogy: Google Maps — "Mirpur theke Gulshan e jete hole Farmgate diye jao"
#include "ns3/ipv4-global-routing-helper.h"

// Node, NetDevice, Packet — basic building blocks
#include "ns3/network-module.h"

// Point-to-point link — duita node er moddhe direct cable
#include "ns3/point-to-point-module.h"


// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0


/* ===================================================================
 *  BLOCK 2: NAMESPACE & LOG DEFINITION
 * ===================================================================
 */
using namespace ns3;

// Ei script er log camera r naam = "SecondScriptExample"
// Define korlei log ON hoy na — LogComponentEnable() lagbe!
NS_LOG_COMPONENT_DEFINE("SecondScriptExample");


/* ===================================================================
 *  BLOCK 3: MAIN FUNCTION + VARIABLES + COMMANDLINE
 * ===================================================================
 *  ── NOTUN vs first.cc! ──
 *  first.cc te CommandLine e kono custom argument chilo na.
 *  Ekhane 2 ta custom variable ache ja terminal theke change korte paro!
 *
 *  Analogy: first.cc chilo fixed recipe — always same.
 *  second.cc te tui recipe r ingredients change korte parcho terminal theke!
 *     ./waf --run "second --nCsma=5 --verbose=false"
 */
int
main(int argc, char* argv[])
{
    // verbose = true mane logging ON thakbe
    // Terminal e --verbose=false dile logging OFF hoye jabe
    bool verbose = true;

    // nCsma = 3 mane CSMA LAN e 3 ta EXTRA node banbe (n2, n3, n4)
    // Terminal e --nCsma=5 dile 5 ta extra node banbe
    // n1 already CSMA te ache (bridge hisebe), ei 3 ta n1 CHARA extra
    uint32_t nCsma = 3;

    CommandLine cmd(__FILE__);

    // Terminal theke nCsma er value change korte dibe
    // Example: ./waf --run "second --nCsma=5"
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);

    // Terminal theke verbose ON/OFF korte dibe
    // Example: ./waf --run "second --verbose=false"
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);

    cmd.Parse(argc, argv);


    /* ===============================================================
     *  BLOCK 4: CONDITIONAL LOGGING
     * ===============================================================
     *  first.cc te logging ALWAYS ON chilo.
     *  Ekhane verbose = true hole ON, false hole OFF.
     *  Terminal e --verbose=false dile → if false → log hobe na → blank screen
     */
    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }


    /* ===============================================================
     *  BLOCK 5: SAFETY GUARD
     * ===============================================================
     *  Jodi keu terminal e --nCsma=0 dey → minimum 1 kore dao
     *  Karon CSMA LAN e at least ekta extra node lage
     *  0 dile code break korte pare
     *
     *  Ternary operator: (condition) ? (true value) : (false value)
     *  nCsma == 0 ? 1 : nCsma
     *    → "nCsma 0 ki? Haan hole 1 koro. Na hole jeta ache setai rakh."
     */
    nCsma = nCsma == 0 ? 1 : nCsma;


    /* ===============================================================
     *  BLOCK 6: NODE CREATE — 2 ta ALAG GROUP! 🖥️
     * ===============================================================
     *
     *  ── GROUP 1: p2pNodes ──
     *  2 ta node: n0, n1 — Point-to-Point link er jonno
     *
     *  ── GROUP 2: csmaNodes ──
     *  n1 + 3 ta extra = [n1, n2, n3, n4] — CSMA LAN er jonno
     *
     *  ⚠️ SHOB CHEYE IMPORTANT POINT:
     *  n1 DUITA group e ache! P2P teo ache, CSMA teo ache!
     *  n1 = BRIDGE / JUNCTION — duita network connect kore!
     *
     *  Analogy: n1 holo MORIR MATHAI er ghor.
     *    Ekdike PRIVATE ROAD (P2P) ache n0 te
     *    Opr dike COLONY ROAD (CSMA LAN) ache n2, n3, n4 te
     *    n1 duita road er JUNCTION point!
     *
     *  Total unique nodes: n0, n1, n2, n3, n4 = 5 ta
     */

    // Group 1: P2P nodes — n0 ar n1
    NodeContainer p2pNodes;
    p2pNodes.Create(2);  // n0 = p2pNodes.Get(0), n1 = p2pNodes.Get(1)

    // Group 2: CSMA nodes — n1 ke add koro + 3 ta EXTRA banao
    NodeContainer csmaNodes;
    csmaNodes.Add(p2pNodes.Get(1));  // n1 ke CSMA group eo add koro (BRIDGE!)
    csmaNodes.Create(nCsma);          // 3 ta extra: n2, n3, n4
    // csmaNodes = [n1, n2, n3, n4]
    //   csmaNodes.Get(0) = n1
    //   csmaNodes.Get(1) = n2
    //   csmaNodes.Get(2) = n3
    //   csmaNodes.Get(3) = n4 ← SERVER ekhane bosbe!


    /* ===============================================================
     *  BLOCK 7: POINT-TO-POINT LINK (n0 ↔ n1) 🔌
     * ===============================================================
     *  Exactly same as first.cc!
     *  n0 ar n1 er moddhe 5Mbps, 2ms direct cable.
     *
     *  P2P te DataRate = DEVICE (NIC) er property
     *    Karon duita NIC INDEPENDENT — each NIC nijer speed e data dhokkay
     *    Analogy: 2 ta ghor er moddhe private pipe — protita TAP er NIJER capacity ache
     *
     *  P2P te Delay = CHANNEL (cable) er property
     *    Karon signal CABLE er vitro diye travel kore — cable koto lomba tar upor depend kore
     *    Analogy: pipe koto lomba — pani travel korte koto time lage
     *
     *  After this:
     *     n0 ───[5Mbps, 2ms P2P]─── n1
     */
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));   // NIC speed
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));       // Cable travel time

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);  // n0, n1 te NIC + cable lagao


    /* ===============================================================
     *  BLOCK 8: CSMA LAN (n1, n2, n3, n4) — NOTUN! 🌐
     * ===============================================================
     *
     *  CSMA = Carrier Sense Multiple Access
     *
     *  Analogy: Meeting room er SHARED MICROPHONE 🎤
     *    - Multiple person (node) EKTA microphone (bus) share kore
     *    - Kotha bolar age SHUNO keu boltechhe ki na (Carrier Sense)
     *    - Keu na bolle → tui bolo (transmit)
     *    - Keu bolchhe → WAIT koro
     *
     *  P2P vs CSMA:
     *  ──────────────────────────────────────────────────
     *  P2P:  2 ta node, direct cable, collision nai
     *  CSMA: Multiple node, SHARED bus, collision possible!
     *  ──────────────────────────────────────────────────
     *
     *  ⚠️ MEGA IMPORTANT — KENO duitai SetChannelAttribute??
     *  ─────────────────────────────────────────────────────
     *
     *  P2P te: DataRate = Device property (each NIC independent)
     *  CSMA te: DataRate = CHANNEL property (shared bus controls speed!)
     *
     *  KENO? Karon CSMA te shob NIC EKTA SHARED BUS e connected!
     *  Bus er speed = 100Mbps → SHOBAI ke 100Mbps e cholte hobe.
     *  Kono NIC bolte pare na "ami 200Mbps e chalbo" — BUS-I 100Mbps!
     *
     *  Analogy (DataRate):
     *    P2P = Duita ghor er moddhe PRIVATE pipe → protita TAP er nijer capacity
     *    CSMA = Colony r SHARED main pipeline → pipeline er capacity = SHOBER limit
     *    Colony r pipe 100 litre/sec hole, tui 200 litre r tap lagalei
     *    pipe 200 litre dibe na — PIPE er limit shober limit!
     *
     *  ⚠️ Delay SHOBSOMOY Channel er property — P2P teo, CSMA teo!
     *  ─────────────────────────────────────────────────────────────
     *  Karon signal SHOBSOMOY cable/bus er VITRO diye travel kore.
     *  NIC shudu data PRODUCE kore ar bus e DHOKKAY.
     *  Data bus e dhukle — baki travel ta BUS er moddhe hoy.
     *  Bus koto lomba → delay koto beshi.
     *
     *  Analogy (Delay):
     *    NIC theke signal berolo → BUS er vitro te dhuklo → BUS er moddhe
     *    diye TRAVEL kortechhe → opr pranto r NIC e pouchchhlo.
     *    Ei TRAVEL time = BUS er physical property (lomba, medium type)
     *    NIC change korleo delay SAME — karon BUS-i same!
     *    Jemon: Ghor 1 r tap change korleo pipe r lomba same thakle
     *           pani pouchchhanor time SAME!
     *
     *  Summary Table:
     *  ──────────────────────────────────────────────────
     *  Attribute  │ P2P te         │ CSMA te        │ Keno
     *  ──────────────────────────────────────────────────
     *  DataRate   │ Device (NIC)   │ Channel (bus)  │ P2P: NIC independent
     *             │                │                │ CSMA: shared bus = shober limit
     *  Delay      │ Channel (cable)│ Channel (bus)  │ DUITATEI Channel!
     *             │                │                │ Signal SHOBSOMOY cable/bus
     *             │                │                │ er vitro diye travel kore
     *  ──────────────────────────────────────────────────
     *
     *  After this:
     *     n1 ──┬── n2
     *          ├── n3
     *          └── n4
     *       CSMA Bus (100Mbps, 6560ns delay)
     */
    CsmaHelper csma;

    // Shared bus er speed = 100Mbps — shobai ei speed e limited
    // P2P chilo 5Mbps — LAN 20 gun faster!
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));

    // Signal bus er vitro diye travel korte 6560 nanoseconds lage
    // = 6.56 microseconds = 0.00656 ms
    // P2P te chilo 2ms — LAN er delay ONEK CHOTO!
    // Karon LAN e devices same room/building e thake — bus khub choto!
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    // n1, n2, n3, n4 shobai ke CSMA bus e connect koro
    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);


    /* ===============================================================
     *  BLOCK 9: INTERNET STACK INSTALL 🌐
     * ===============================================================
     *  ⚠️ CAREFUL! Alag alag install kora hocche — karon n1 DUITA group e!
     *
     *  stack.Install(p2pNodes.Get(0)) → shudu n0 te install
     *  stack.Install(csmaNodes)       → n1, n2, n3, n4 te install
     *
     *  KENO alada alada?
     *  Karon n1 already csmaNodes er part!
     *  csmaNodes = [n1, n2, n3, n4]
     *  So stack.Install(csmaNodes) e n1 o cover hoye jay.
     *
     *  ⚠️ EXAM TRAP:
     *  Jodi stack.Install(p2pNodes) likhti → n0 ar N1 DUITATEI install hoto
     *  Then stack.Install(csmaNodes) e n1 te DUIBAR install hoto → ERROR!
     *  Same node e DUIBAR stack install KORTE PARBI NA!
     *
     *  Tai: n0 → alada install
     *       n1,n2,n3,n4 → eksathe install (csmaNodes diye)
     */
    InternetStackHelper stack;
    stack.Install(p2pNodes.Get(0));  // shudu n0
    stack.Install(csmaNodes);        // n1, n2, n3, n4 (n1 ekhane cover hocche)


    /* ===============================================================
     *  BLOCK 10: IP ADDRESS ASSIGN — 2 ta ALAG NETWORK! 📍
     * ===============================================================
     *
     *  ── Network 1: P2P (10.1.1.0/24) ──
     *  n0 er P2P NIC  → 10.1.1.1
     *  n1 er P2P NIC  → 10.1.1.2
     *
     *  ── Network 2: CSMA (10.1.2.0/24) ──
     *  n1 er CSMA NIC → 10.1.2.1
     *  n2 er CSMA NIC → 10.1.2.2
     *  n3 er CSMA NIC → 10.1.2.3
     *  n4 er CSMA NIC → 10.1.2.4
     *
     *  ⚠️ n1 er 2 ta NIC, 2 ta alag IP!
     *  P2P NIC = 10.1.1.2, CSMA NIC = 10.1.2.1
     *  Jemon tomar phone e WiFi o ache (192.168.1.5),
     *  mobile data o ache (10.45.3.7) — 2 ta alag connection, 2 ta alag IP!
     *
     *  Full picture:
     *       10.1.1.1             10.1.1.2 | 10.1.2.1     10.1.2.2  .3    .4
     *     n0 ──────[P2P]────── n1 ────────┬──── n2 ──── n3 ──── n4
     *                                     │     CSMA Bus
     */

    // ── Network 1: P2P ──
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");  // Network: 10.1.1.0/24
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);
    // p2pInterfaces.GetAddress(0) = 10.1.1.1 (n0)
    // p2pInterfaces.GetAddress(1) = 10.1.1.2 (n1 er P2P NIC)

    // ── Network 2: CSMA ──
    address.SetBase("10.1.2.0", "255.255.255.0");  // Network: 10.1.2.0/24
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);
    // csmaInterfaces.GetAddress(0) = 10.1.2.1 (n1 er CSMA NIC)
    // csmaInterfaces.GetAddress(1) = 10.1.2.2 (n2)
    // csmaInterfaces.GetAddress(2) = 10.1.2.3 (n3)
    // csmaInterfaces.GetAddress(3) = 10.1.2.4 (n4) ← SERVER er IP!


    /* ===============================================================
     *  BLOCK 11: SERVER APPLICATION (n4 te) 🖥️
     * ===============================================================
     *  UDP Echo Server — protidhwoni server
     *  Ja ashbe → EXACT SAME fire pathabe. Kono modify na!
     *
     *  Server kothay? csmaNodes.Get(nCsma) = csmaNodes.Get(3) = n4
     *  csmaNodes = [n1, n2, n3, n4] → index 3 = n4 = SHESH node!
     *
     *  Server n4 te, port 9 e listen kortechhe.
     *  Start: 1s, Stop: 10s
     */
    UdpEchoServerHelper echoServer(9);  // Port 9 e server

    // csmaNodes.Get(nCsma) = csmaNodes.Get(3) = n4
    ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(nCsma));
    serverApps.Start(Seconds(1));   // 1s e server ON
    serverApps.Stop(Seconds(10));   // 10s e server OFF


    /* ===============================================================
     *  BLOCK 12: CLIENT APPLICATION (n0 te) 📱
     * ===============================================================
     *  Client n0 te boshe ache.
     *  Destination: n4 er IP = csmaInterfaces.GetAddress(nCsma)
     *             = csmaInterfaces.GetAddress(3) = 10.1.2.4
     *
     *  n0 (10.1.1.1) → n4 (10.1.2.4) — ALAG NETWORK!
     *  Packet journey: n0 → P2P → n1 (bridge) → CSMA → n4
     *
     *  Ekhane ROUTING lagbe — n0 ke jante hobe
     *  "10.1.2.4 e jete hole n1 (10.1.1.2) diye jao"
     *  Seta next block e solve hobe (PopulateRoutingTables)
     */

    // Destination: n4 er CSMA IP (10.1.2.4), port 9
    UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(nCsma), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));      // 1 ta packet
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1)));   // 1s gap (irrelevant for 1 packet)
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));   // 1024 bytes = 1 KB

    // Client n0 te install
    ApplicationContainer clientApps = echoClient.Install(p2pNodes.Get(0));
    clientApps.Start(Seconds(2));   // 2s e client ON → packet pathay
    clientApps.Stop(Seconds(10));   // 10s e client OFF


    /* ===============================================================
     *  BLOCK 13: ROUTING — NOTUN! 🗺️
     * ===============================================================
     *  ⚠️ SUPER IMPORTANT! first.cc te ei line CHILO NA!
     *
     *  KENO first.cc te lageni?
     *    first.cc: shudu 2 node, same network (10.1.1.0) — direct connected
     *    Routing er dorkar chilo na.
     *
     *  KENO ekhane lagchhe?
     *    n0 ache 10.1.1.0 network e
     *    n4 ache 10.1.2.0 network e — ALAG NETWORK!
     *    n0 ke jante hobe "10.1.2.4 e jete hole ki korte hobe?"
     *
     *  PopulateRoutingTables() AUTOMATICALLY shob node er routing table
     *  fill kore dey:
     *
     *    n0 er routing table:
     *    ──────────────────────────────────────────────────
     *    Destination     │ Next Hop      │ Interface
     *    ──────────────────────────────────────────────────
     *    10.1.1.0/24     │ direct        │ P2P NIC
     *    10.1.2.0/24     │ 10.1.1.2 (n1) │ P2P NIC
     *    ──────────────────────────────────────────────────
     *    "CSMA network (10.1.2.0) e jete hole n1 diye jao!"
     *
     *  Analogy: Google Maps route calculate kore deoa
     *    Tui Mirpur e achho (10.1.1.0), jete chao Gulshan e (10.1.2.0)
     *    Maps bole: "Farmgate (n1) diye jao!"
     *
     *  ⚠️ EXAM TRAP: Eta na likhle n0 bujhbe na 10.1.2.4 e kivabe jete hobe
     *     → packet DROP hobe → server KICHHU pabe na!
     */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    /* ===============================================================
     *  BLOCK 14: PCAP PACKET CAPTURE — NOTUN! 📦
     * ===============================================================
     *  PCAP = Packet Capture
     *  Network e ki packet jacche shob RECORD kore — Wireshark diye
     *  pore open kore dekhte parbi.
     *
     *  Analogy: Network e CCTV camera lagachhish.
     *  PCAP files = CCTV footage. Pore Wireshark diye playback korbi.
     *
     *  EnablePcapAll("second"):
     *    P2P link er SHOB node er capture kore
     *    Files banbe:
     *      second-0-0.pcap (n0 er P2P NIC)
     *      second-1-0.pcap (n1 er P2P NIC)
     *
     *  EnablePcap("second", csmaDevices.Get(1), true):
     *    CSMA er specific device — csmaDevices.Get(1) = n2 er NIC
     *    true = PROMISCUOUS MODE
     *    Promiscuous mane: shudu nijer packet na, LAN er SHOB packet capture!
     *    Karon CSMA shared bus — shobai shob dekhte pare.
     *    Analogy: Meeting room e shob kotha SHONA jay — promiscuous = shob record koro
     */
    pointToPoint.EnablePcapAll("second");
    csma.EnablePcap("second", csmaDevices.Get(1), true);  // n2 er NIC, promiscuous


    /* ===============================================================
     *  BLOCK 15: RUN SIMULATION 🚀
     * ===============================================================
     *
     *  FULL PACKET JOURNEY:
     *  ─────────────────────────────────────────────────────────
     *  Time 1s:   Server START (n4, port 9 e listening)
     *
     *  Time 2s:   Client START (n0)
     *             n0 → packet banay, destination: 10.1.2.4
     *             n0 routing table check → "10.1.2.0? n1 diye jao!"
     *             n0 ──[P2P 5Mbps, 2ms]──→ n1
     *
     *             n1 receive kore → destination 10.1.2.4 dekhe
     *             n1 → "eta amar CSMA network e! Forward!"
     *             n1 ──[CSMA 100Mbps]──→ n4
     *
     *             n4 server packet pay → ECHO kore fire pathay!
     *
     *             n4 ──[CSMA]──→ n1 ──[P2P]──→ n0
     *
     *             n0 echo reply pay → "Network kaaj kortechhe!" ✅
     *
     *  Time 10s:  Both apps stop
     *  ─────────────────────────────────────────────────────────
     */
    Simulator::Run();       // SIMULATION START! Shob event execute hoy
    Simulator::Destroy();   // Simulation shesh — memory cleanup
    return 0;               // Program successfully end
}


/*
 * ===================================================================
 *  BUILD ORDER — ei ORDER e code likha hoyeche:
 * ===================================================================
 *  1. Create P2P nodes (n0, n1)
 *  2. Create CSMA nodes (n1 + n2, n3, n4)     ← n1 shared!
 *  3. Install P2P link (n0 ↔ n1)
 *  4. Install CSMA LAN (n1, n2, n3, n4)
 *  5. Install TCP/IP stack (n0 alada, csmaNodes eksathe)
 *  6. Assign IPs (2 ta alag network)
 *  7. Install Server (n4) + Client (n0)
 *  8. Populate routing tables                  ← NOTUN!
 *  9. Enable PCAP capture                      ← NOTUN!
 *  10. Run simulation
 *
 *  ⚠️ Stack n1 te DUIBAR install korle ERROR!
 *     Tai p2pNodes.Get(0) alada, csmaNodes eksathe.
 * ===================================================================
 *
 *  first.cc vs second.cc — KEY DIFFERENCES:
 *  ──────────────────────────────────────────────────
 *  Feature          │ first.cc       │ second.cc
 *  ──────────────────────────────────────────────────
 *  Networks         │ 1 (P2P)        │ 2 (P2P + CSMA)
 *  Nodes            │ 2 (n0,n1)      │ 5 (n0-n4)
 *  New module       │ None           │ CSMA + Routing
 *  Routing needed?  │ No             │ YES!
 *  CommandLine args │ None           │ nCsma, verbose
 *  PCAP capture     │ No             │ Yes
 *  n1 er role       │ Just endpoint  │ BRIDGE!
 *  n1 er NIC count  │ 1              │ 2 (P2P + CSMA)
 *  n1 er IP count   │ 1 (10.1.1.2)   │ 2 (10.1.1.2 + 10.1.2.1)
 *  ──────────────────────────────────────────────────
 *
 *  QUICK EXAM QUESTIONS:
 *  ─────────────────────
 *  Q: CSMA te DataRate keno Channel er property, Device er na?
 *  A: CSMA te shob NIC EKTA shared bus e connected. Bus er speed =
 *     shober limit. Kono NIC bus er cheye faster jete pare na.
 *     Analogy: Colony r main pipe 100 litre/sec hole, 200 litre tap
 *     lagalei pipe 200 dibe na!
 *
 *  Q: Delay keno SHOBSOMOY Channel er property (P2P teo CSMA teo)?
 *  A: Signal SHOBSOMOY cable/bus er VITRO diye travel kore.
 *     NIC shudu data produce kore ar bus e dhokkay.
 *     Data bus e dhukle baki travel BUS er moddhe hoy.
 *     Bus er lomba = delay. NIC change korleo delay same!
 *
 *  Q: PopulateRoutingTables() na likhle ki hobe?
 *  A: n0 bujhbe na 10.1.2.4 e kivabe jete hobe → packet DROP!
 *
 *  Q: stack.Install(p2pNodes) ar stack.Install(csmaNodes) duitai korle?
 *  A: n1 te DUIBAR install hobe → ERROR! Same node e duibar stack
 *     install korte parbi na!
 *
 *  Q: csmaNodes.Get(3) mane ki?
 *  A: csmaNodes = [n1, n2, n3, n4]. Index 3 = n4 = server node.
 *
 *  Q: n1 er koyта IP ache?
 *  A: 2 ta! P2P NIC: 10.1.1.2, CSMA NIC: 10.1.2.1
 * ===================================================================
 */