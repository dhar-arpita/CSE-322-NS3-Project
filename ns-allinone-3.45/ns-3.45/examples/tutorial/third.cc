/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * ===================================================================
 *  third.cc — NS-3 er THIRD example script
 *  CSE 322: Computer Networks
 * ===================================================================
 *
 *  PURA CODE KI KORCHHE? (Big Picture)
 *  ------------------------------------
 *  TINTA ALAG network banacche ar connect koreche:
 *
 *     WiFi 10.1.3.0          P2P 10.1.1.0         CSMA 10.1.2.0
 *                   AP
 *    *    *    *    *
 *    |    |    |    |
 *   n5   n6   n7   n0 ──────────────── n1 ───┬─── n2
 *   (WiFi STAs)   (AP + Bridge) 5Mbps,2ms    ├─── n3
 *                                      (Bridge)└─── n4 (Server)
 *                                            CSMA Bus (100Mbps)
 *
 *  LEFT:   WiFi (10.1.3.0) — n5,n6,n7 = wireless stations, n0 = AP
 *  MIDDLE: P2P (10.1.1.0)  — n0 ↔ n1, direct cable
 *  RIGHT:  CSMA (10.1.2.0) — n1,n2,n3,n4 shared bus
 *
 *  Client = n7 (WiFi STA — shesh WiFi node)
 *  Server = n4 (CSMA LAN er shesh node)
 *
 *  PACKET JOURNEY:
 *  n7 →(wireless)→ n0(AP) →(P2P cable)→ n1 →(CSMA bus)→ n4
 *  n4 echo kore → n1 → n0 →(wireless)→ n7
 *
 *  BRIDGES:
 *  n0 = DOUBLE BRIDGE! WiFi AP + P2P cable (2 ta NIC, 2 ta IP)
 *       Analogy: Bashar WiFi router — ekdike phone wireless,
 *       opr dike ISP er cable. Router duita duniya connect kore!
 *  n1 = BRIDGE! P2P cable + CSMA bus (2 ta NIC, 2 ta IP)
 *
 *  Keno n1 e P2P ar CSMA DUITAI install?
 *  → n1 ke 2 ta ALAG network CONNECT korte hobe!
 *    P2P NIC diye n0 theke packet ASHCHHE
 *    CSMA NIC diye sei packet CSMA LAN e JACCHHE
 *    Jodi CSMA NIC na thaktoh → n1 CSMA LAN e dhukteii partoh na!
 *    Jodi P2P NIC na thaktoh → n0 theke packet n1 e pouchchhotoi na!
 *    Same concept n0 teo — WiFi NIC diye n7 theke ashchhe,
 *    P2P NIC diye n1 er dike jacche.
 *    Analogy: Tomar laptop e WiFi NIC + Ethernet NIC thake —
 *    laptop internally decide kore konta NIC diye kothay pathabe.
 *
 *  EVOLUTION:
 *  ─────────────────────────────────────────────────
 *  first.cc:  1 network, 2 nodes, no routing
 *  second.cc: 2 networks, 5 nodes, routing needed
 *  third.cc:  3 networks, 8 nodes, routing + WiFi + mobility!
 *  ─────────────────────────────────────────────────
 *
 *  NOTUN concepts in third.cc (vs second.cc):
 *    1. WiFi network (STA + AP + SSID)
 *    2. Mobility models (nodes ghure beray!)
 *    3. Simulator::Stop() — mobility never stops tai force stop lagey
 *    4. Conditional tracing (--tracing=true flag)
 * ===================================================================
 */


/* ===================================================================
 *  BLOCK 1: HEADER FILES (#include)
 * ===================================================================
 *  second.cc te 7 ta chilo, ekhane 9 ta — 3 ta NOTUN:
 *
 *  Analogy reminder (from first.cc):
 *    core-module     = Engine (NS-3 cholbei na chara)
 *    network-module  = Body/Chassis (Node, Packet)
 *    internet-module = GPS system (TCP/IP, routing)
 *    p2p-module      = Duita ghor er moddhe private pipe
 *    csma-module     = Colony r shared pipeline
 *    applications    = Passenger (actual kaajer jinish)
 *
 *  NOTUN 3 ta:
 *    mobility-module    = Node der GPS tracker (position + movement)
 *    ssid.h             = WiFi er naam tag
 *    yans-wifi-helper.h = WiFi radio setup
 */

#include "ns3/applications-module.h"  // UdpEchoClient, UdpEchoServer
#include "ns3/core-module.h"          // Simulator, Time, CommandLine, Logging
#include "ns3/csma-module.h"          // CSMA LAN — shared bus, meeting room er shared mic

#include "ns3/internet-module.h"      // TCP/IP stack, IP addressing

// ── NOTUN! ──
// Node der position ar movement set korte lagbe.
// WiFi WIRELESS — node physically kothay ache seta signal strength ke affect kore!
// Tui router theke dure gele signal weak hoy, kaachhe gele strong.
// Wired network e mobility lage na — karon cable er length fixed.
//
// NS-3 te onek mobility model ache:
// ─────────────────────────────────────────────────────────────────
// Model                    │ Movement style          │ Analogy
// ─────────────────────────────────────────────────────────────────
// ConstantPositionModel    │ MOVE KORE NA             │ WiFi router (fixed desk e)
// ConstantVelocityModel    │ Straight line, same speed│ Highway gari, cruise control
// RandomWalk2dModel        │ Chaotic random every step│ Maathal manush rastar upor
// RandomWaypointModel      │ Random dest, straight,   │ Shopping mall — shop to shop
//                          │ pause, repeat            │
// RandomDirection2dModel   │ Straight to boundary,    │ Bouncing ball
//                          │ bounce                   │
// GaussMarkovModel         │ Smooth gradual change    │ Ship samudra te
// WaypointModel            │ YOU define exact path    │ GPS navigation, planned route
// ─────────────────────────────────────────────────────────────────
// Ei code e RandomWalk2d (STA) ar ConstantPosition (AP) use hoyeche.
#include "ns3/mobility-module.h"

#include "ns3/network-module.h"       // Node, NetDevice, Packet

#include "ns3/point-to-point-module.h" // P2P link — direct cable

// ── NOTUN! ──
// SSID = Service Set Identifier = WiFi network er NAAM
// Jemon tomar phone e WiFi list dekhish — "Home_WiFi", "TP-Link_5G"
// Shob STA ar AP ke SAME SSID dite hobe — tahole same network e connect hobe
#include "ns3/ssid.h"

// ── NOTUN! ──
// YANS = "Yet Another Network Simulator"
// WiFi er physical layer (radio channel, signal) setup korte lagbe
// Wired network e radio channel lage na — WiFi te radio waves use hoy tai extra step
// Analogy: FM radio te 98.4 MHz te tune koro — WiFi teo radio channel e tune kore
#include "ns3/yans-wifi-helper.h"


// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0


/* ===================================================================
 *  BLOCK 2: NAMESPACE & LOG DEFINITION
 * ===================================================================
 */
using namespace ns3;

// Ei script er log camera r naam = "ThirdScriptExample"
// Define korlei log ON hoy na — LogComponentEnable() lagbe!
// Registration ≠ Activation!
// Analogy: CCTV camera ke naam dili, but ekhono record button dabao ni.
NS_LOG_COMPONENT_DEFINE("ThirdScriptExample");


/* ===================================================================
 *  BLOCK 3: MAIN FUNCTION + VARIABLES + COMMANDLINE
 * ===================================================================
 *  second.cc theke 2 ta NOTUN variable:
 *    nWifi   = WiFi STA node count
 *    tracing = PCAP capture ON/OFF
 *
 *  Terminal theke customize korte paro:
 *    ./waf --run "third --nCsma=5 --nWifi=4 --verbose=false --tracing=true"
 */
int
main(int argc, char* argv[])
{
    bool verbose = true;       // Logging ON/OFF. Default ON.
    uint32_t nCsma = 3;       // CSMA LAN e koyта EXTRA node. Default 3 (n2,n3,n4)
    uint32_t nWifi = 3;        // WiFi STA koyта node. Default 3 (n5,n6,n7) ← NOTUN!
    bool tracing = false;      // PCAP capture ON/OFF. Default OFF.          ← NOTUN!
    // Tracing ki?
    // Tracing = Network e ki hocche shob RECORD kora — jate pore ANALYZE korte paro.
    // Analogy: Network er CCTV camera!
    //   tracing = false → CCTV OFF. Simulation cholbe but kichhu record hobe na.
    //   tracing = true  → CCTV ON. Protita packet .pcap file e record hobe.
    //                     Pore Wireshark diye open kore shob detail dekhte parbi.
    //
    // NS-3 te 2 rokom tracing ache:
    // ─────────────────────────────────────────────────
    // Type       │ File     │ Open with   │ Analogy
    // ─────────────────────────────────────────────────
    // PCAP       │ .pcap    │ Wireshark   │ Video CCTV footage
    //            │          │ (GUI)       │ (protita packet er shob detail)
    // ASCII      │ .tr      │ Notepad/cat │ Written logbook diary
    //            │          │ (text)      │ (+,−,r,d events text e)
    // ─────────────────────────────────────────────────
    // + = packet queue e dhuklo (parcel counter e joma dili)
    // - = packet queue theke berolo (parcel van e uthlo)
    // r = packet receive holo (parcel pouchchhe gelo)
    // d = packet DROP holo! (parcel haariye gelo!)
    //
    // Amader code e shudu PCAP tracing ache (ASCII nai).

    CommandLine cmd(__FILE__);
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    cmd.AddValue("nWifi", "Number of wifi STA devices", nWifi);   // ← NOTUN!
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue("tracing", "Enable pcap tracing", tracing);      // ← NOTUN!

    cmd.Parse(argc, argv);


    /* ===============================================================
     *  BLOCK 4: SAFETY CHECK — nWifi MAX 18
     * ===============================================================
     *  WiFi STA nodes ke ekta GRID layout e position deoa hoy (pore dekhbi).
     *  Grid er bounding box fixed (-50 to 50). 18 er beshi node dile
     *  grid er moddhe jagah thake na — nodes baire chole jay.
     *  Tai safety check: max 18.
     */
    if (nWifi > 18)
    {
        std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box"
                  << std::endl;
        return 1;  // Program exit — simulation cholbe na
    }


    /* ===============================================================
     *  BLOCK 5: CONDITIONAL LOGGING
     * ===============================================================
     *  verbose = true hole log ON, false hole OFF.
     *  Terminal e --verbose=false dile → if false → blank screen.
     *
     *  Log Levels (jemon volume control):
     *    LOG_LEVEL_ERROR = shudu errors (Volume 1)
     *    LOG_LEVEL_WARN  = errors + warnings (Volume 2)
     *    LOG_LEVEL_INFO  = important info (Volume 3) ← ETA use hocche
     *    LOG_LEVEL_DEBUG = detailed debug (Volume 4)
     *    LOG_LEVEL_ALL   = SHOB KICHHU (MAX VOLUME)
     */
    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }


    /* ===============================================================
     *  BLOCK 6: P2P NODES + LINK (Same as second.cc)
     * ===============================================================
     *  n0 ar n1 — Point-to-Point direct cable
     *
     *  P2P te:
     *    DataRate = DEVICE (NIC) er property — each NIC independent
     *      Analogy: 2 ta ghor er private pipe — protita TAP er nijer capacity
     *    Delay = CHANNEL (cable) er property — signal cable diye travel kore
     *      Analogy: pipe koto lomba — pani travel korte koto time
     *
     *  Transmission Delay vs Propagation Delay:
     *  ─────────────────────────────────────────────────────
     *  Transmission = Data NIC theke cable e DHUKATE koto time
     *    = PacketSize / DataRate = (1024×8)/5Mbps ≈ 1.6ms
     *    Analogy: Tap theke baalti bhora
     *  Propagation = Signal cable diye TRAVEL korte koto time
     *    = 2ms (fixed, cable er property)
     *    Analogy: Pipe diye pani travel kora
     *  Total ≈ 1.6ms + 2ms = 3.6ms
     *  ─────────────────────────────────────────────────────
     */
    NodeContainer p2pNodes;
    p2pNodes.Create(2);  // n0, n1

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));  // NIC speed
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));      // Cable travel time

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);  // NIC + cable lagao
    // After: n0 ───[5Mbps, 2ms P2P]─── n1


    /* ===============================================================
     *  BLOCK 7: CSMA NODES + LAN (Same as second.cc)
     * ===============================================================
     *  n1 ke CSMA group eo add — n1 = BRIDGE (P2P + CSMA e connected)
     *  n2, n3, n4 = extra CSMA nodes
     *
     *  CSMA = Carrier Sense Multiple Access
     *  Analogy: Meeting room er SHARED MICROPHONE 🎤
     *    Multiple person EKTA mic share kore
     *    Bolar age SHUNO keu boltechhe ki na → na bolle → bolo
     *
     *  ⚠️ KENO duitai SetChannelAttribute?
     *  ─────────────────────────────────────────────────────
     *  Attribute │ P2P te         │ CSMA te        │ Keno
     *  ─────────────────────────────────────────────────────
     *  DataRate  │ Device (NIC)   │ Channel (bus)  │ CSMA: shared bus = shober limit
     *            │                │                │ Colony pipe 100L/sec hole
     *            │                │                │ 200L tap lagalei 200 dibe na!
     *  Delay     │ Channel (cable)│ Channel (bus)  │ DUITATEI Channel!
     *            │                │                │ Signal SHOBSOMOY cable/bus
     *            │                │                │ er VITRO diye travel kore
     *            │                │                │ NIC shudu data produce kore
     *            │                │                │ Bus e dhukle baki travel
     *            │                │                │ BUS er moddhe hoy!
     *  ─────────────────────────────────────────────────────
     */
    NodeContainer csmaNodes;
    csmaNodes.Add(p2pNodes.Get(1));  // n1 ke CSMA group eo add (BRIDGE!)
    csmaNodes.Create(nCsma);          // n2, n3, n4
    // csmaNodes = [n1, n2, n3, n4]

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));           // Bus speed
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));        // Bus travel time
    // 6560 ns = 0.00656 ms — ONEK choto! Karon LAN e devices same room e thake

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);  // n1,n2,n3,n4 CSMA bus e connect
    // After: n1 ──┬── n2 ── n3 ── n4
    //          CSMA Bus (100Mbps)


    /* ===============================================================
     *  BLOCK 8: WiFi NODES CREATE — NOTUN! 📡
     * ===============================================================
     *  WiFi network e 2 rokom device thake:
     *
     *  STA (Station) = WiFi CLIENT device
     *    Tomar phone/laptop — ja WiFi e connect kore
     *    Ekhane: n5, n6, n7
     *
     *  AP (Access Point) = WiFi ROUTER
     *    Tomar bashar WiFi router — signal broadcast kore, STA der connect kore
     *    Ekhane: n0 ← already P2P teo ache — ekhon WiFi AP O hobe!
     *
     *  n0 er ekhon DOUBLE BRIDGE role:
     *    WiFi AP ←→ [n0] ←→ P2P cable ←→ n1
     *    (wireless)         (wired)
     *    Analogy: Bashar WiFi router — ekdike phone wireless,
     *    opr dike ISP er cable lagano. Router duita duniya connect kore!
     *
     *             📡 n0 (AP)
     *            / | \
     *          /   |   \  wireless signal
     *        📱   📱   📱
     *        n5   n6   n7 (STAs)
     */
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(nWifi);  // 3 ta STA: n5, n6, n7

    // n0 ke WiFi AP banao — p2pNodes.Get(0) = n0
    // n0 already P2P group e ache — ekhon WiFi AP o hobe (DOUBLE BRIDGE!)
    NodeContainer wifiApNode = p2pNodes.Get(0);


    /* ===============================================================
     *  BLOCK 9: WiFi PHYSICAL LAYER (Channel + PHY) — NOTUN! 📻
     * ===============================================================
     *  WiFi WIRELESS — tai radio channel setup korte hoy!
     *  Wired network e ei step CHILO NA — wire e radio channel lage na.
     *
     *  Analogy: FM radio te 98.4 MHz te tune koro —
     *  WiFi teo ekta radio channel e tune kore. Ei lines seta setup kore.
     *
     *  YansWifiChannelHelper → WiFi er radio channel banay
     *    Default() → default propagation loss model + delay model
     *
     *  YansWifiPhyHelper → Physical layer — radio signal kemon hobe, power koto
     *
     *  phy.SetChannel() → PHY ke channel er sathe connect koro
     *    "Ei channel diye signal pathao/receive koro"
     */
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());


    /* ===============================================================
     *  BLOCK 10: WiFi MAC LAYER — STA + AP Setup — NOTUN! 🔧
     * ===============================================================
     *
     *  SSID = WiFi er naam. Jemon "Home_WiFi" — ekhane "ns-3-ssid".
     *  Shob STA ar AP ke SAME SSID dite hobe — tahole same network.
     *
     *  STA ar AP er MAC behavior ALAG:
     *    StaWifiMac = CLIENT behavior (connect kore AP te)
     *    ApWifiMac  = ROUTER behavior (broadcast kore, STA der manage kore)
     */
    WifiMacHelper mac;

    // WiFi er naam = "ns-3-ssid"
    // Jemon tomar phone e "Home_WiFi" dekhish WiFi list e
    Ssid ssid = Ssid("ns-3-ssid");

    WifiHelper wifi;

    // ── STA (Station) devices setup ──
    // StaWifiMac = WiFi CLIENT behavior
    // ActiveProbing = false → STA nijei AP khundhbe na
    //   AP er broadcast beacon shunbe ar connect korbe
    //   true hole STA actively probe request pathabe AP khojar jonno
    // Analogy: Phone er WiFi ON → phone dekhchhe "ns-3-ssid" ache → connect!
    NetDeviceContainer staDevices;
    mac.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid),
                "ActiveProbing", BooleanValue(false));
    staDevices = wifi.Install(phy, mac, wifiStaNodes);  // n5, n6, n7 te WiFi STA NIC

    // ── AP (Access Point) device setup ──
    // ApWifiMac = WiFi ROUTER behavior
    // "ns-3-ssid" naam e network broadcast kortechhe
    // Analogy: WiFi router ON → "ns-3-ssid" signal broadcast hocche
    NetDeviceContainer apDevices;
    mac.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid));
    apDevices = wifi.Install(phy, mac, wifiApNode);  // n0 te WiFi AP NIC


    /* ===============================================================
     *  BLOCK 11: MOBILITY — NOTUN! 🚶
     * ===============================================================
     *  WiFi WIRELESS — node physically kothay ache seta signal ke affect kore!
     *  Router theke dure gele signal weak, kaachhe gele strong.
     *  NS-3 ke jante hobe — node KOTHAY ache ar KOTHAY JACCHHE.
     *
     *  Wired network e mobility lage na — cable er length fixed.
     *  WiFi te MUST lagey!
     *
     *  Ei code e 2 ta model use hocche:
     *    STA (n5,n6,n7) → RandomWalk2dMobilityModel (ghure beray)
     *    AP (n0)         → ConstantPositionMobilityModel (fixed thake)
     *
     *  Keno AP fixed? Real life eo WiFi router ekjayga e thake!
     *  Tui router niye ghurish na — phone niye ghurish!
     */
    MobilityHelper mobility;

    // ── Part 1: Grid Position Allocator ──
    // STA nodes er STARTING position set kore — ekta grid e boshao
    //
    // GridWidth = 3, DeltaX = 5m, DeltaY = 10m, Start (0,0)
    //
    //   n5(0,0)    n6(5,0)    n7(10,0)    ← Row 1
    //      ← 5m →    ← 5m →
    //
    // 3 ta node, GridWidth=3 → shobai 1 row te
    // Jodi 6 ta node thaktoh → 2 rows:
    //   Row 1: (0,0)  (5,0)  (10,0)
    //   Row 2: (0,10) (5,10) (10,10)    ← DeltaY = 10m niche
    //
    // Analogy: Class e bench e boshano — 3 jon ek row te, 5 feet gap e
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),      // Grid start X
                                  "MinY", DoubleValue(0.0),      // Grid start Y
                                  "DeltaX", DoubleValue(5.0),    // X gap between nodes = 5m
                                  "DeltaY", DoubleValue(10.0),   // Y gap between rows = 10m
                                  "GridWidth", UintegerValue(3),  // 3 nodes per row
                                  "LayoutType", StringValue("RowFirst"));  // Row age bhoro

    // ── Part 2: STA nodes GHURE BERAY! ──
    // RandomWalk2dMobilityModel:
    //   Node RANDOM direction e haate — kono destination nai
    //   Boundary te thokke direction reverse hoy
    //   Analogy: Maathal manush rastar upor — jemon khosha temon haate
    //
    // Bounds = (-50, 50, -50, 50) → 100m × 100m box er moddhe
    //   Baire jete parbe na!
    //
    //   (-50,50)────────────────(50,50)
    //       │  n5→   ←n7          │
    //       │      n6↓            │
    //       │                     │
    //   (-50,-50)───────────────(50,-50)
    //        100m × 100m bounding box
    //
    // Other mobility models available:
    //   RandomWaypointModel → random dest e jay, pause, repeat (shopping mall)
    //   ConstantVelocityModel → soja same speed (highway gari)
    //   GaussMarkovModel → smooth gradual direction change (ship)
    //   WaypointModel → TUI define koro exact path (GPS navigation)
    //   RandomDirection2dModel → straight to boundary, bounce (bouncing ball)
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds",
                              RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility.Install(wifiStaNodes);  // n5, n6, n7 ghure beray

    // ── Part 3: AP node STHIR thake! ──
    // ConstantPositionMobilityModel:
    //   Node EKDOM nore na. Fixed position.
    //   Analogy: WiFi router desk er upor fixed — keu sorate pare na
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode);  // n0 (AP) fixed


    /* ===============================================================
     *  BLOCK 12: INTERNET STACK INSTALL 🌐
     * ===============================================================
     *  ⚠️ CAREFULLY alag alag install — karon n0, n1 multiple group e!
     *
     *  csmaNodes = [n1, n2, n3, n4] → n1 ekhane cover hoy
     *  wifiApNode = n0 → alada install
     *  wifiStaNodes = [n5, n6, n7]
     *
     *  ⚠️ EXAM TRAP:
     *  stack.Install(p2pNodes) korle n0+n1 duitatei install hoto
     *  Then csmaNodes e n1 te DUIBAR install → ERROR!
     *  Same node e duibar stack install KORTE PARBI NA!
     *  Tai carefully:
     *    n1 → csmaNodes er part e cover hocche
     *    n0 → alada wifiApNode diye install hocche
     */
    InternetStackHelper stack;
    stack.Install(csmaNodes);       // n1, n2, n3, n4 (n1 ekhane cover)
    stack.Install(wifiApNode);      // n0 alada
    stack.Install(wifiStaNodes);    // n5, n6, n7


    /* ===============================================================
     *  BLOCK 13: IP ADDRESS ASSIGN — 3 ta ALAG NETWORK! 📍
     * ===============================================================
     *
     *  Network 1 — P2P (10.1.1.0/24):
     *    n0 P2P NIC  → 10.1.1.1
     *    n1 P2P NIC  → 10.1.1.2
     *
     *  Network 2 — CSMA (10.1.2.0/24):
     *    n1 CSMA NIC → 10.1.2.1
     *    n2          → 10.1.2.2
     *    n3          → 10.1.2.3
     *    n4          → 10.1.2.4 ← SERVER!
     *
     *  Network 3 — WiFi (10.1.3.0/24):
     *    n5 STA → 10.1.3.1
     *    n6 STA → 10.1.3.2
     *    n7 STA → 10.1.3.3 ← CLIENT!
     *    n0 AP  → 10.1.3.4 (approx — assign order e depend kore)
     *
     *  ⚠️ n0 er 2 ta NIC, 2 ta IP! (P2P: 10.1.1.1, WiFi: 10.1.3.x)
     *  ⚠️ n1 er 2 ta NIC, 2 ta IP! (P2P: 10.1.1.2, CSMA: 10.1.2.1)
     *
     *  Analogy: Phone e WiFi (192.168.1.5) + Mobile data (10.45.3.7)
     *  — 2 ta alag connection, 2 ta alag IP!
     */
    Ipv4AddressHelper address;

    // Network 1: P2P
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    // Network 2: CSMA
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);

    // Network 3: WiFi
    address.SetBase("10.1.3.0", "255.255.255.0");
    address.Assign(staDevices);   // n5, n6, n7 er WiFi NIC
    address.Assign(apDevices);    // n0 er WiFi AP NIC


    /* ===============================================================
     *  BLOCK 14: SERVER APPLICATION (n4) 🖥️
     * ===============================================================
     *  Same as second.cc — n4 te UDP Echo Server, port 9
     *  Echo = Protidhwoni — ja ashbe exact same fire pathabe!
     *  Analogy: Pahar er shamne "HELLO!" chillale "HELLO!" fire ashbe
     *
     *  Echo er purpose = TESTING!
     *    Packet pouchchhe ki na? ✅   RTT measure? ✅
     *    Packet corrupt? ✅           Network alive? ✅
     */
    UdpEchoServerHelper echoServer(9);

    // csmaNodes.Get(nCsma) = csmaNodes.Get(3) = n4
    ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(nCsma));
    serverApps.Start(Seconds(1));   // 1s e ON
    serverApps.Stop(Seconds(10));   // 10s e OFF


    /* ===============================================================
     *  BLOCK 15: CLIENT APPLICATION (n7!) 📱
     * ===============================================================
     *  ── NOTUN vs second.cc! ──
     *  second.cc: client = n0 (P2P side)
     *  third.cc:  client = n7 (WiFi STA!)
     *
     *  wifiStaNodes.Get(nWifi - 1) = wifiStaNodes.Get(2) = n7
     *
     *  Destination: n4 er IP = csmaInterfaces.GetAddress(nCsma) = 10.1.2.4
     *
     *  Packet journey — 3 ta network cross kore!
     *  n7 →(wireless)→ n0(AP) →(P2P 5Mbps)→ n1 →(CSMA 100Mbps)→ n4
     *  10.1.3.3       10.1.1.1           10.1.1.2/10.1.2.1       10.1.2.4
     */
    UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(nCsma), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));       // 1 ta packet
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1)));    // (irrelevant for 1)
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));    // 1024 bytes = 1 KB

    // Client = n7 (WiFi STA er shesh node)
    ApplicationContainer clientApps = echoClient.Install(wifiStaNodes.Get(nWifi - 1));
    clientApps.Start(Seconds(2));   // 2s e ON
    clientApps.Stop(Seconds(10));   // 10s e OFF


    /* ===============================================================
     *  BLOCK 16: ROUTING 🗺️
     * ===============================================================
     *  3 ta ALAG network — routing MUST!
     *  n7 ke jante hobe "10.1.2.4 e jete hole n0 → n1 diye jao"
     *
     *  PopulateRoutingTables() automatically shob node er routing table
     *  fill kore dey.
     *
     *  Analogy: Google Maps — "Mirpur theke Gulshan, Farmgate diye jao!"
     *
     *  ⚠️ Eta na likhle packet DROP — kono node janbei na onno network
     *     e kivabe pouchchhabe!
     */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    /* ===============================================================
     *  BLOCK 17: SIMULATOR STOP — NOTUN! ⏱️
     * ===============================================================
     *  ⚠️ first.cc ar second.cc te ei line CHILO NA!
     *
     *  KENO ekhane lagchhe?
     *  WiFi STA nodes CONTINUOUSLY ghure beracche (RandomWalk).
     *  Tader mobility model KABHU stop hoy na!
     *  Wired network e shob sthir — apps stop korle simulator shesh.
     *  But WiFi te mobility ALWAYS RUNNING.
     *
     *  ⚠️ Eta na likhle simulation KABHU SHESH HOBENA — infinite e ghumbe!
     *
     *  Simulator::Stop(Seconds(10)) bole:
     *  "10 second e FORCEFULLY shob bondho koro — mobility choluk ba na choluk!"
     */
    Simulator::Stop(Seconds(10));


    /* ===============================================================
     *  BLOCK 18: PCAP TRACING (Conditional) — NOTUN! 📦
     * ===============================================================
     *  second.cc te PCAP ALWAYS ON chilo.
     *  Ekhane tracing flag TRUE hole i ON hobe.
     *  Default OFF — karon 3 ta network er capture = onek boro .pcap files!
     *
     *  Tracing er kaaj ki?
     *  ───────────────────────────────────────────────────────
     *  1. DEBUGGING — "keno kaaj kortechhe na?"
     *     Wireshark e dekhi kothay packet drop hochhe
     *     "ARP reply ashchena!" → routing bhul → FIX!
     *
     *  2. PERFORMANCE ANALYSIS — "koto fast?"
     *     Wireshark e delay measure — WiFi te 3ms, P2P te 2ms, CSMA 2ms
     *     "WiFi te delay beshi — optimize korte hobe!"
     *
     *  3. PROTOCOL ANALYSIS — "internally ki hocche?"
     *     ARP exchange koybar? Retry koybar? Collision holo ki na?
     *  ───────────────────────────────────────────────────────
     *
     *  Terminal e ON korte: ./waf --run "third --tracing=true"
     */
    if (tracing)
    {
        // WiFi packet e 802.11 radio header add koro
        // Wireshark e WiFi signal strength, data rate dekhte parbi
        phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

        // P2P link er SHOB node er packet capture
        // Files: third-0-0.pcap (n0 P2P), third-1-0.pcap (n1 P2P)
        pointToPoint.EnablePcapAll("third");

        // WiFi AP (n0) er packet capture
        // Wireless e ki ashchhe jacche dekhte parbi
        phy.EnablePcap("third", apDevices.Get(0));

        // CSMA LAN (n1 er CSMA NIC) capture — PROMISCUOUS mode
        // Promiscuous = shudu nijer packet na, LAN er SHOB packet capture!
        // CSMA shared bus — shobai shob shune — promiscuous = shob record koro
        // Analogy: Meeting room e shob kotha SHONA jay — shob record!
        csma.EnablePcap("third", csmaDevices.Get(0), true);
    }
    // tracing = false hole ei pura block SKIP
    // Kono .pcap file banbe na — but simulation THIK E cholbe!
    // Analogy: CCTV OFF — office cholchhe, shudu footage nai.


    /* ===============================================================
     *  BLOCK 19: RUN SIMULATION 🚀
     * ===============================================================
     *
     *  FULL PACKET JOURNEY:
     *  ─────────────────────────────────────────────────────────
     *  Time 1s:   Server START (n4, port 9 listening)
     *
     *  Time 2s:   Client START (n7 — WiFi STA)
     *             n7 packet banay, destination: 10.1.2.4 (n4)
     *
     *             STEP 1: n7 →(wireless)→ n0 (AP)
     *             WiFi radio diye n0 er kachhe pathay
     *
     *             STEP 2: n0 →(P2P 5Mbps, 2ms)→ n1
     *             n0 P2P cable diye n1 te forward kore
     *
     *             STEP 3: n1 →(CSMA 100Mbps)→ n4
     *             n1 CSMA bus diye n4 te forward kore
     *
     *             STEP 4: n4 server packet pay → ECHO kore!
     *             Protidhwoni — exact same packet fire pathay!
     *
     *             STEP 5: Return journey
     *             n4 →(CSMA)→ n1 →(P2P)→ n0 →(wireless)→ n7
     *
     *             n7: "Reply pelam! Network kaaj kortechhe!" ✅
     *
     *  Time 10s:  Simulator::Stop() forces everything to end
     *  ─────────────────────────────────────────────────────────
     */
    Simulator::Run();       // SIMULATION START!
    Simulator::Destroy();   // Memory cleanup
    return 0;
}


/*
 * ===================================================================
 *  BUILD ORDER:
 * ===================================================================
 *  1.  Create P2P nodes (n0, n1)
 *  2.  Install P2P link (n0 ↔ n1)
 *  3.  Create CSMA nodes (n1 + n2,n3,n4)
 *  4.  Install CSMA LAN
 *  5.  Create WiFi STA nodes (n5,n6,n7)              ← NOTUN
 *  6.  Create WiFi AP (n0)                            ← NOTUN
 *  7.  Setup WiFi PHY + Channel                       ← NOTUN
 *  8.  Setup WiFi MAC (STA + AP) + Install            ← NOTUN
 *  9.  Setup Mobility (RandomWalk for STA, Fixed AP)  ← NOTUN
 *  10. Install TCP/IP stack (carefully — no duplicates!)
 *  11. Assign IPs (3 networks)
 *  12. Install Server (n4) + Client (n7)
 *  13. Populate routing tables
 *  14. Simulator::Stop(10s)                           ← NOTUN
 *  15. Optional PCAP tracing
 *  16. Run!
 * ===================================================================
 *
 *  second.cc vs third.cc — KEY DIFFERENCES:
 *  ──────────────────────────────────────────────────
 *  Feature          │ second.cc        │ third.cc
 *  ──────────────────────────────────────────────────
 *  Networks         │ 2 (P2P+CSMA)     │ 3 (WiFi+P2P+CSMA)
 *  Total nodes      │ 5 (n0-n4)        │ 8 (n0-n7)
 *  New module       │ CSMA+Routing     │ WiFi+Mobility!
 *  Client           │ n0 (P2P)         │ n7 (WiFi STA!)
 *  Wireless?        │ No               │ Yes!
 *  Mobility?        │ No               │ Yes! STA ghure beray
 *  Simulator::Stop? │ No               │ Yes (mobility never stops)
 *  n0 er role       │ P2P endpoint     │ WiFi AP + P2P bridge!
 *  n0 er NIC count  │ 1 (P2P)          │ 2 (P2P + WiFi AP)
 *  PCAP             │ Always ON        │ Conditional (--tracing)
 *  New variables    │ nCsma,verbose    │ nCsma,nWifi,verbose,tracing
 *  ──────────────────────────────────────────────────
 *
 *  QUICK EXAM QUESTIONS:
 *  ─────────────────────
 *  Q: Simulator::Stop(Seconds(10)) keno lagchhe?
 *  A: WiFi STA er RandomWalk mobility KABHU stop hoy na!
 *     Explicitly stop na korle simulation shesh hobena — infinite!
 *
 *  Q: n0 er koyта NIC ache?
 *  A: 2 ta! P2P NIC (10.1.1.1) + WiFi AP NIC (10.1.3.x)
 *
 *  Q: ActiveProbing = false mane ki?
 *  A: STA nijei AP khundhbe na. AP er beacon shunbe → connect korbe.
 *
 *  Q: nWifi max 18 keno?
 *  A: Grid layout er bounding box fixed. 18+ hole nodes baire jay.
 *
 *  Q: Tracing OFF hole simulation e ki effect?
 *  A: KONO effect nai! Simulation same cholbe — shudu .pcap file banbe na.
 *
 *  Q: RandomWalk vs RandomWaypoint er difference?
 *  A: RandomWalk = chaotic direction change every step (maathal manush)
 *     RandomWaypoint = random dest, straight line, PAUSE, repeat (mall)
 *
 *  Q: CSMA te DataRate keno Channel er property?
 *  A: Shared bus — bus speed = shober limit. Colony pipe 100L/sec hole
 *     200L tap lagalei 200 dibe na!
 *
 *  Q: Delay keno SHOBSOMOY Channel er property?
 *  A: Signal cable/bus er VITRO diye travel kore — NIC shudu produce kore.
 * ===================================================================
 */