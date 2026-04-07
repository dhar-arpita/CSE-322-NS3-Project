/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * ===================================================================
 *  first.cc — NS-3 er FIRST example script
 *  CSE 322: Computer Networks
 * ===================================================================
 *
 *  PURA CODE KI KORCHHE? (Big Picture)
 *  ------------------------------------
 *  Ekta KHUB simple network banacche:
 *
 *         10.1.1.1              10.1.1.2
 *     n0 ──────────────────────── n1
 *     (Client)  5Mbps, 2ms    (Server)
 *               Point-to-Point
 *
 *  n0 (Client) ekta 1024 byte packet pathay n1 (Server) ke
 *  n1 (Server) EXACT SAME packet ta echo (protidhwoni) kore fire pathay
 *  Basically: "Hello? Shuntechho?" test — network thik ache ki na check!
 *
 *  TIMELINE:
 *     0s   → Simulation starts (kichhu hoy na)
 *     1s   → Server START (port 9 e listen shuru)
 *     2s   → Client START → packet pathay n1 er port 9 e
 *     ~2.003s → Server packet pay → SAME packet echo kore fire pathay
 *     ~2.006s → Client echo reply pay → "Network kaaj kortechhe!" ✅
 *     10s  → Both apps stop → Simulation end
 * ===================================================================
 */


/* ===================================================================
 *  BLOCK 1: HEADER FILES (#include)
 * ===================================================================
 *  Jemon C++ e #include <iostream> likhi cout use korar jonno,
 *  temni NS-3 er different features use korte different header lage.
 *
 *  Analogy: Tui ekta gari banachhish —
 *    core-module     = Engine (chara gari cholbei na)
 *    network-module  = Body/Chassis (gari r structure)
 *    internet-module = GPS system (route/IP jante pare)
 *    p2p-module      = Duita ghor er moddhe pipe line
 *    applications    = Passenger (actual kaajer jinish)
 */

// NS-3 er engine — Simulator, Time, CommandLine, Logging
// Eta chara NS-3 cholbei na!
#include "ns3/core-module.h"

// Node (computer), NetDevice (NIC/LAN card), Packet — basic building blocks
// Jemon gari r body/chassis
#include "ns3/network-module.h"

// TCP/IP stack, IP address assign kora
// Jemon gari te GPS lagano — jate route bujhte pare
#include "ns3/internet-module.h"

// Point-to-point link — duita node er moddhe direct cable connection
// Jemon duita ghor er moddhe ekta pipe line
#include "ns3/point-to-point-module.h"

// UdpEchoClient, UdpEchoServer — actual apps ja data pathay/receive kore
// Jemon gari te passenger bosano — actual kaajer jinish
#include "ns3/applications-module.h"


// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//


/* ===================================================================
 *  BLOCK 2: NAMESPACE & LOG DEFINITION
 * ===================================================================
 */

// NS-3 er shob class ns3:: namespace e thake.
// Eta likhle ns3:: bar bar likhte hobe na.
// Jemon: ns3::NodeContainer er bodole just NodeContainer likhleii cholbe.
using namespace ns3;

// Ei script ke ekta NAAM diye dichhish logging system e — "FirstScriptExample"
//
// Analogy: School e 100 ta CCTV camera ache — protitar ekta naam.
//   Eta holo tomar script er "camera naam" — pore tui bolte parbi
//   "FirstScriptExample er log dekhao" ba "bondho koro"
//
// NOTE: Eta shudu NAAM REGISTRATION — eta log ON kore na!
//   LogComponentEnable() call korle log ON hoy.
//   Registration ≠ Activation!
NS_LOG_COMPONENT_DEFINE("FirstScriptExample");


/* ===================================================================
 *  BLOCK 3: MAIN FUNCTION START
 * ===================================================================
 */
int
main(int argc, char* argv[])
{
    // CommandLine parser — terminal theke argument nite dey
    // Example: ./waf --run "first --DataRate=10Mbps"
    // Ekhane kono custom argument nai, but structure rakhা hoyeche future er jonno
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);


    /* ===============================================================
     *  BLOCK 4: TIME RESOLUTION & LOGGING ENABLE
     * ===============================================================
     */

    // Simulator er time precision NANOSECOND e set kora holo (10^-9 second)
    // Mane simulator ekdom precise time track korbe.
    // Other options: Time::US (microsecond), Time::MS (millisecond)
    Time::SetResolution(Time::NS);

    // Client app er logging ON kora holo — INFO level e
    // Etar fole terminal e dekhbi:
    //   "At time +2s client sent 1024 bytes to 10.1.1.2 port 9"
    //
    // Log Levels (jemon volume control):
    //   LOG_LEVEL_ERROR = shudu errors (Volume 1)
    //   LOG_LEVEL_WARN  = errors + warnings (Volume 2)
    //   LOG_LEVEL_INFO  = errors + warnings + important info (Volume 3) ← ETA use hocche
    //   LOG_LEVEL_DEBUG = shob + detailed debug info (Volume 4)
    //   LOG_LEVEL_ALL   = SHOB KICHHU (MAX VOLUME)
    //
    // ⚠️ EXAM TRAP: Ei 2 line comment out korle simulation THIK E cholbe,
    //    but terminal e KONO output dekhbi na! Blank screen!
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);


    /* ===============================================================
     *  BLOCK 5: NODE CREATE 🖥️
     * ===============================================================
     *  Node = ekta blank computer. Just hardware box — kono NIC nai,
     *  kono IP nai, kono software nai, kono cable nai.
     *
     *  nodes.Create(2) er por state:
     *     [n0] (empty)          [n1] (empty)
     *
     *  nodes.Get(0) = n0 (first node)
     *  nodes.Get(1) = n1 (second node)
     *  Indexing 0 theke start!
     *
     *  Analogy: 2 ta KHALI computer kina anلام — ekhono kichhu install kora hoy ni
     */
    NodeContainer nodes;  // Container (like array) ja nodes hold korbe
    nodes.Create(2);      // 2 ta blank node banao: n0 ar n1


    /* ===============================================================
     *  BLOCK 6: POINT-TO-POINT LINK CREATE 🔌
     * ===============================================================
     *  Ekhane 2 ta ALAG jinish configure hocche:
     *
     *  [n0]---[NIC]---[=======CABLE=======]---[NIC]---[n1]
     *          ↑              ↑                 ↑
     *        DEVICE         CHANNEL           DEVICE
     *
     *  DEVICE (NIC) = Network Card — computer er moddhe
     *  CHANNEL (Cable) = Duita NIC er moddhe je cable/taR ache
     *
     *  Analogy (Water System):
     *    [Ghor A]---[TAP/কল]---[====PIPE====]---[TAP/কল]---[Ghor B]
     *                 ↑             ↑               ↑
     *               DEVICE       CHANNEL          DEVICE
     *    TAP = koto jore pani berobe (DataRate)
     *    PIPE = pani pouchchhte koto time lage (Delay)
     */
    PointToPointHelper pointToPoint;  // Helper object — link setup korbe

    // ── SetDeviceAttribute: NIC er property set kore ──
    // DataRate = 5Mbps → NIC 1 second e 5 Megabits data process korte pare
    // Analogy: TAP (কল) second e 5 litre pani release korte pare — TAP er capacity
    // Ei ta holo BANDWIDTH — pipe er MOTAI
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));

    // ── SetChannelAttribute: CABLE er property set kore ──
    // Delay = 2ms → Signal cable diye ekpranto theke opr pranto te jete 2ms lage
    // Analogy: PIPE 10 meter lomba — pani ekdik theke opr dike jete 2 second lage
    // Ei ta holo PROPAGATION DELAY — pipe er LOMBA
    //
    // ⚠️ IMPORTANT: DataRate ≠ Delay!
    //   DataRate = NIC koto fast data process kore (bandwidth/motai)
    //   Delay = Signal cable diye jete koto time lage (latency/lomba)
    //   Duitai independently packet delivery te affect kore!
    //
    // Packet delivery time = Transmission time + Propagation time
    //   Transmission = PacketSize/DataRate = (1024×8)/5000000 ≈ 1.6ms
    //   Propagation = Delay = 2ms
    //   Total ≈ 3.6ms
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    // Install() — EKTA line e 3 ta kaam!
    //   1. n0 te ekta NIC (NetDevice) lagalo ✅
    //   2. n1 teo ekta NIC (NetDevice) lagalo ✅
    //   3. Duitar moddhe cable (channel) connect korlo ✅
    //
    // ⚠️ Install() shudu NIC + cable lagay — IP address ba TCP/IP dey na!
    //
    // Ekhon state:
    //   [n0 + NIC] ───cable─── [n1 + NIC]
    //   (ekhono IP nai, TCP/IP nai)
    //
    // Analogy: Duita computer e LAN card lagaiya cable diye connect korlam.
    //          But ekhono networking software install kori ni!
    NetDeviceContainer devices;          // NIC gula store korar container
    devices = pointToPoint.Install(nodes); // NIC lagao + cable connect koro


    /* ===============================================================
     *  BLOCK 7: INTERNET STACK INSTALL 🌐
     * ===============================================================
     *  Duita node e TCP/IP protocol stack install korchhe:
     *    - IP layer
     *    - TCP / UDP
     *    - ARP, ICMP, etc.
     *
     *  Analogy: Computer e LAN card lagaiyechhilam (Block 6),
     *           ekhon Windows/Linux er "networking software" install korlam.
     *           Tahole IP address dite parbo, TCP/UDP use korte parbo.
     *
     *  Ekhon state:
     *    [n0 + NIC + TCP/IP] ───cable─── [n1 + NIC + TCP/IP]
     *    (ekhono IP address nai!)
     *
     *  ⚠️ EXAM TRAP: IP assign korte hole AAGE stack install thakte hobe!
     *     Stack chara IP assign korar infrastructure-i nai!
     *     Order: Create nodes → Install NIC → Install Stack → THEN assign IP
     */
    InternetStackHelper stack;  // Helper object — TCP/IP install korbe
    stack.Install(nodes);       // n0 ar n1 DUITATEI TCP/IP install koro


    /* ===============================================================
     *  BLOCK 8: IP ADDRESS ASSIGN 📍
     * ===============================================================
     *  Network: 10.1.1.0 / 255.255.255.0 (i.e., /24)
     *
     *  NS-3 automatically sequential IP assign kore:
     *    n0 er NIC → 10.1.1.1
     *    n1 er NIC → 10.1.1.2
     *
     *  .0 holo network address (assign hoy na)
     *  .255 holo broadcast address (assign hoy na)
     *  .1 theke host address shuru hoy
     *
     *  Ekhon state — NETWORK FULLY READY! ✅
     *    10.1.1.1              10.1.1.2
     *    [n0 + NIC + TCP/IP] ───cable─── [n1 + NIC + TCP/IP]
     *
     *  But ekhono kono APPLICATION choltechhe na — keu data pathacche na!
     */
    Ipv4AddressHelper address;                  // Helper object — IP assign korbe
    address.SetBase("10.1.1.0", "255.255.255.0"); // Network: 10.1.1.0/24

    // IP assign kore NIC gula ke, ar result store kore interfaces e
    // interfaces.GetAddress(0) = 10.1.1.1 (n0 er IP)
    // interfaces.GetAddress(1) = 10.1.1.2 (n1 er IP)
    //
    // ⚠️ EXAM TRAP: GetAddress(1) = n1 er IP = 10.1.1.2
    //    GetAddress(0) likhle n0 er OWN IP pabi — client nijeke nijei pathabe!
    Ipv4InterfaceContainer interfaces = address.Assign(devices);


    /* ===============================================================
     *  BLOCK 9: SERVER APPLICATION (n1 te) 🖥️
     * ===============================================================
     *  UDP Echo Server = Protidhwoni (প্রতিধ্বনি) server
     *  Ja ashbe → EXACT SAME jinish fire pathabe. Kono modify na!
     *
     *  Analogy: Pahar er shamne chillachhish "HELLOOO!"
     *           Pahar fire pathay "HELLOOO!" — exact same!
     *
     *  Keno Echo use kori? Real data transfer to na!
     *  → TESTING er jonno! Network thik ache ki na check korar jonno.
     *    - Packet pouchchhe ki na? ✅
     *    - Koto time lagchhe (RTT)? ✅
     *    - Packet corrupt hochhe ki na? ✅
     *
     *  Jemon tui phone kinei first call e bolish:
     *  "Hello? Shuntechho? Hello bolo toh!" — echo = sei test!
     *
     *  ⚠️ EXAM TRAP: Server AAGE start hoy (1s), Client PORE (2s).
     *     Server ke age ready thakte hobe! Na hole client packet pathabe,
     *     but receive korar keu thakbe na → packet LOST!
     */

    // Port 9 e UDP Echo Server banao
    // Port ki? Computer er "room number" — client ke jante hobe "port 9 e pathao"
    UdpEchoServerHelper echoServer(9);

    // Server ke n1 te (nodes.Get(1) = second node) install koro
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(Seconds(1));   // 1 second e server ON → port 9 e listening shuru
    serverApps.Stop(Seconds(10));   // 10 second e server OFF


    /* ===============================================================
     *  BLOCK 10: CLIENT APPLICATION (n0 te) 📱
     * ===============================================================
     *  Client er kaaj:
     *    1. n1 er IP (10.1.1.2) er port 9 e packet pathabe
     *    2. Server echo reply pathale, receive korbe
     *    3. Done! Network test complete!
     *
     *  n0 (Client)                              n1 (Server)
     *      |                                        |
     *      |   1s: Server starts listening           |
     *      |                                    [Port 9: READY]
     *      |                                        |
     *      |   2s: Client sends 1024 bytes          |
     *      |────────── UDP Packet ────────────────→ |
     *      |                                        |
     *      |   ~2.003s: Server echoes back          |
     *      | ←──────── SAME UDP Packet ────────────|
     *      |                                        |
     *      |   ~2.006s: Client receives echo        |
     *      |   "Network kaaj kortechhe!" ✅          |
     */

    // Client banao — destination: n1 er IP (10.1.1.2), port 9
    // interfaces.GetAddress(1) = 10.1.1.2 = n1 er IP
    // ⚠️ GetAddress(0) likhle client NIJEKE NIJEI packet pathabe (n0 → n0)!
    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);

    // MaxPackets = 1 → Total 1 ta packet pathabe. Bas, ekhane job done.
    // Jodi 3 kori → 3 ta packet pathabe (2s, 3s, 4s e — Interval onujayee)
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));

    // Interval = 1 second → Packets er moddhe 1s gap
    // Ekhane just 1 packet, tai Interval matter kore na.
    // But MaxPackets 3 hole: 2s e first, 3s e second, 4s e third
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1)));

    // PacketSize = 1024 bytes → Protita packet 1024 bytes (1 KB) size er
    // 512 korle half size er packet jabe, 2048 korle double
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    // Client ke n0 te (nodes.Get(0) = first node) install koro
    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2));   // 2 second e client ON → immediately packet pathay
    clientApps.Stop(Seconds(10));   // 10 second e client OFF


    /* ===============================================================
     *  BLOCK 11: RUN SIMULATION 🚀
     * ===============================================================
     *  Simulator::Run()  → Shob scheduled events time order e execute hoy
     *  Simulator::Destroy() → Memory cleanup
     *
     *  ⚠️ Run() na likhle KICHHU hobe na — nodes create hoye
     *     program exit korbe, kono packet pathabe na!
     *
     *  ⚠️ Destroy() na likhle simulation cholbe but memory leak hobe.
     *     Good practice = always call Destroy()
     *
     *  EXPECTED OUTPUT (terminal e):
     *  ─────────────────────────────────────────────────────────────
     *  At time +2s client sent 1024 bytes to 10.1.1.2 port 9
     *  At time +2.00369s server received 1024 bytes from 10.1.1.1 port 49153
     *  At time +2.00369s server sent 1024 bytes to 10.1.1.1 port 49153
     *  At time +2.00737s client received 1024 bytes from 10.1.1.2 port 9
     *  ─────────────────────────────────────────────────────────────
     *  Eta dekhbi KARON LogComponentEnable ON kora chilo!
     *  ON na thakle → blank screen, but simulation internally hoto.
     */
    Simulator::Run();       // SIMULATION START! Shob event execute hoy
    Simulator::Destroy();   // Simulation shesh — memory cleanup
    return 0;               // Program successfully end
}


/*
 * ===================================================================
 *  BUILD ORDER — EI ORDER MUST MAINTAIN korte hobe!
 * ===================================================================
 *
 *  Step 1: Create Nodes       (nodes.Create)         → Khali computer banao
 *  Step 2: Install NIC+Cable  (pointToPoint.Install)  → LAN card + cable lagao
 *  Step 3: Install TCP/IP     (stack.Install)          → Networking software install
 *  Step 4: Assign IP          (address.Assign)         → IP address dao
 *  Step 5: Install Apps       (server + client)        → Application install koro
 *  Step 6: Run Simulation     (Simulator::Run)         → CHOLAO!
 *
 *  ⚠️ Ulta korle crash/error!
 *     IP assign korte hole AAGE stack install thakte hobe!
 *     App install korte hole AAGE IP thakte hobe!
 * ===================================================================
 *
 *  QUICK EXAM QUESTIONS:
 *  ─────────────────────
 *  Q: LogComponentEnable comment out korle?
 *  A: Simulation cholbe but kono output terminal e dekhbi na
 *
 *  Q: Client Seconds(0), Server Seconds(2) korle?
 *  A: Client age pathabe but Server ready na → packet LOST!
 *
 *  Q: MaxPackets 5 korle?
 *  A: 5 ta packet: 2s, 3s, 4s, 5s, 6s e (1s interval e)
 *
 *  Q: nodes.Create(2) er por n0 te ki ache?
 *  A: KICHHU NAI — just blank node, no NIC, no IP, no stack
 *
 *  Q: SetDeviceAttribute vs SetChannelAttribute?
 *  A: Device = NIC er property (DataRate/bandwidth/pipe er motai)
 *     Channel = Cable er property (Delay/propagation/pipe er lomba)
 *
 *  Q: interfaces.GetAddress(0) client e dile?
 *  A: Client nijeke nijei pathabe (10.1.1.1 → 10.1.1.1)!
 *
 *  Q: Echo Server ki kore?
 *  A: Ja pabe EXACT SAME fire pathabe. Protidhwoni! Kono modify na.
 * ===================================================================
 */