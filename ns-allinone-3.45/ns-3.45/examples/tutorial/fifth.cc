/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * ===================================================================
 *  fifth.cc — NS-3 er FIFTH example script
 *  CSE 322: Computer Networks
 * ===================================================================
 *
 *  PURA CODE KI KORCHHE? (Big Picture)
 *  ------------------------------------
 *  first.cc er moto simple 2-node P2P network —
 *  BUT ebar UDP na, TCP!
 *
 *         10.1.1.1              10.1.1.2
 *    n0 ──────────────────────── n1
 *    (TCP Sender)  5Mbps, 2ms  (TCP Receiver)
 *    (TutorialApp)              (PacketSink)
 *    1000 packets               Shudu receive kore
 *    each 1040 bytes            echo kore NA!
 *
 *  Ki hocche:
 *    1. n0 continuously TCP data pathacche n1 ke (1000 packets)
 *    2. n1 te PacketSink — data receive kore (echo kore na!)
 *    3. TCP er Congestion Window (cwnd) TRACE hocche
 *       → cwnd change hole terminal e print hoy
 *    4. n1 te Error Model — randomly packet DROP kore!
 *       → TCP ke recover korte hoy → cwnd kome!
 *
 *  ═══════════════════════════════════════════════════════
 *  TCP Congestion Window (cwnd) ki?
 *  ═══════════════════════════════════════════════════════
 *  cwnd = TCP ekbar e koyта packet pathate pare
 *         bina ACK er jonno wait na kore
 *
 *  Analogy: Courier service!
 *    Shuru te: "Ekbar e 1 ta parcel pathao. ACK pele arekta."  cwnd=1
 *    Aste aste: "2 ta pathao!" cwnd=2, "4 ta!" cwnd=4, "8 ta!" cwnd=8
 *    SUDDENLY parcel HAARIYE GELO (packet loss)! 😱
 *    "SLOW DOWN! 4 ta e namao!" cwnd=4
 *    Abar aste aste barao...
 *
 *  Ei cwnd er ura-naama = SAWTOOTH pattern!
 *    cwnd
 *      │      ╱╲
 *      │     ╱  ╲         ╱╲
 *      │    ╱    ╲       ╱  ╲
 *      │   ╱      ╲     ╱
 *      │  ╱        ╲   ╱
 *      │ ╱          ╲ ╱
 *      │╱            ╳
 *      └──────────────────── time
 *           ↑ loss!    ↑ loss!
 *  ═══════════════════════════════════════════════════════
 *
 *  fourth.cc vs fifth.cc:
 *  ─────────────────────────────────────────────────
 *  Feature         │ fourth.cc           │ fifth.cc
 *  ─────────────────────────────────────────────────
 *  Network ache?   │ NAI                 │ HAA! P2P
 *  Ki trace kore?  │ Simple integer      │ TCP cwnd!
 *  Real app?       │ Na — concept demo   │ HAA! TCP flow
 *  TracedValue     │ Shikhechilam        │ APPLY kortechhi!
 *  ─────────────────────────────────────────────────
 *
 *  first.cc vs fifth.cc:
 *  ─────────────────────────────────────────────────
 *  Feature         │ first.cc            │ fifth.cc
 *  ─────────────────────────────────────────────────
 *  Protocol        │ UDP                 │ TCP!
 *  Server          │ UdpEchoServer       │ PacketSink
 *                  │ (echo back)         │ (just receive, dustbin!)
 *  Client          │ UdpEchoClient       │ TutorialApp (custom)
 *  Packets         │ 1                   │ 1000!
 *  Tracing         │ None                │ cwnd + drop!
 *  Error model     │ None                │ Yes! Random drop
 *  Congestion ctrl │ N/A (UDP has none)  │ TCP NewReno!
 *  Subnet          │ /24                 │ /30
 *  ─────────────────────────────────────────────────
 * ===================================================================
 */


/* ===================================================================
 *  BLOCK 1: HEADER FILES
 * ===================================================================
 */

// ── NOTUN! ──
// Custom application class — TutorialApp
// NS-3 er built-in OnOff app USE KORTE PARI NA karon:
//   1. OnOff app er socket App Start AAGE create hoy na
//      → tai aage theke cwnd trace connect korte pari na
//   2. Socket private — access korte pari na
// Tai nijer simple app baniyechi — socket ke nijei control kori
#include "tutorial-app.h"

#include "ns3/applications-module.h"   // PacketSink etc.
#include "ns3/core-module.h"           // Simulator, Time, Config
#include "ns3/internet-module.h"       // TCP/IP stack, Socket
#include "ns3/network-module.h"        // Node, NetDevice, Packet
#include "ns3/point-to-point-module.h" // P2P link

// File I/O — data file e write korar jonno (available, explicitly use hocche na)
#include <fstream>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FifthScriptExample");


// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |    ns-3 TCP    |    |    ns-3 TCP    |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                5 Mbps, 2 ms
//
// ===========================================================================


/* ===================================================================
 *  BLOCK 2: CALLBACK FUNCTIONS — 2 ta! 🔔
 * ===================================================================
 *  fourth.cc te ekta callback chilo (IntTrace).
 *  Ekhane DUITA — ekta cwnd er jonno, ekta packet drop er jonno.
 *
 *  Ei functions MANUALLY call korte hoy NA!
 *  TracedValue NIJEI call kore AUTOMATICALLY!
 *  (fourth.cc te shikhechilam ei concept — alarm system!)
 */

/**
 * ── CALLBACK 1: CwndChange ──
 * TCP er Congestion Window (cwnd) jokhon change hoy
 * → ei function AUTOMATICALLY call hoy!
 *
 * Output example: "2.5    15360"
 *                  time   new cwnd value
 *
 * Ei output diye GRAPH banate paro!
 *   X-axis = time (seconds)
 *   Y-axis = cwnd (bytes)
 *   → SAWTOOTH pattern dekhbe!
 *
 * NS_LOG_UNCOND ki?
 *   "UNCONDITIONAL log" — LogComponentEnable na korleo print hobe!
 *   Normal NS_LOG_INFO er jonno enable korte hoy.
 *   NS_LOG_UNCOND SHOBSOMOY print hobe.
 *
 * Simulator::Now().GetSeconds()
 *   → Simulation er EKHONKAR time ta seconds e return kore
 *
 * Analogy: TCP er speedometer r alarm —
 *   speed (cwnd) change hole notification ashey!
 *   "2.5 second e cwnd 15360 bytes hoyeche!"
 */
static void
CwndChange(uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << newCwnd);
}

/**
 * ── CALLBACK 2: RxDrop ──
 * Receiver (n1) e jodi kono packet physically DROP hoy
 * (error model er karone) → ei function AUTO call hoy!
 *
 * Output example: "RxDrop at 3.72501"
 *
 * Analogy: Parcel haariye gele notification —
 *   "3.72 second e ekta parcel haariye gechhe!"
 *
 * Ei drop er por TCP detect korbe "loss hoyeche!"
 *   → cwnd HALF hoye jabe (SAWTOOTH er downward slope)
 */
static void
RxDrop(Ptr<const Packet> p)
{
    NS_LOG_UNCOND("RxDrop at " << Simulator::Now().GetSeconds());
}


/* ===================================================================
 *  BLOCK 3: MAIN FUNCTION
 * ===================================================================
 */
int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);


    /* ===============================================================
     *  BLOCK 4: TCP CONFIGURATION — NOTUN! ⚙️
     * ===============================================================
     *  Config::SetDefault ki kore?
     *    Pura simulation er jonno DEFAULT value set kore.
     *    Joto TCP socket banbe — shob e ei settings apply hobe.
     *    Analogy: Shob phone er default ringtone set kora —
     *    protiTA phone e same ringtone hobe.
     *
     *  3 ta TCP setting configure hocche:
     */

    // ── Setting 1: TCP Algorithm = NewReno ──
    // Congestion Control Algorithm set kore.
    // Remember Advanced Topics slides theke —
    //   Linux e CUBIC (default), BBR (Google er), Reno (old) chilo?
    //   NS-3 teo different algorithm choose korte paro!
    //
    // NewReno ki kore?
    //   Slow Start → cwnd EXPONENTIALLY barey (1,2,4,8,16...)
    //   Congestion Avoidance → cwnd LINEARLY barey (16,17,18...)
    //   Packet loss hole → cwnd HALF kore (18→9)
    //   Fast Recovery → quickly recover korar try kore
    //
    // Analogy: Courier company r traffic management policy —
    //   "Slow e shuru koro, aste aste barao, jam lagley slow koro"
    Config::SetDefault("ns3::TcpL4Protocol::SocketType",
                       StringValue("ns3::TcpNewReno"));

    // ── Setting 2: Initial cwnd = 1 packet ──
    // TCP connection SHURU hobe cwnd = 1 packet diye
    // Mane shuru te ekbar e 1 tai packet pathabe
    // Pore aste aste baRbe (slow start)
    Config::SetDefault("ns3::TcpSocket::InitialCwnd",
                       UintegerValue(1));

    // ── Setting 3: Recovery = Classic ──
    // Packet loss er por kivabe recover korbe — Classic algorithm use
    Config::SetDefault("ns3::TcpL4Protocol::RecoveryType",
                       TypeIdValue(TypeId::LookupByName("ns3::TcpClassicRecovery")));


    /* ===============================================================
     *  BLOCK 5: NODES + P2P LINK (Same as first.cc)
     * ===============================================================
     *  n0 ↔ n1, 5Mbps DataRate, 2ms Delay
     *
     *  Reminder:
     *    DataRate = Device (NIC) er property — NIC koto fast data process kore
     *      Analogy: Tap er capacity (pipe er motai)
     *    Delay = Channel (cable) er property — signal travel time
     *      Analogy: Pipe er lomba
     */
    NodeContainer nodes;
    nodes.Create(2);   // n0 (sender), n1 (receiver)

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);


    /* ===============================================================
     *  BLOCK 6: ERROR MODEL — NOTUN! 💥
     * ===============================================================
     *  n1 (receiver) er NIC te ekta Error Model lagacchhi.
     *  Eta randomly kichhu packet DROP kore!
     *  Jemon real network e packet corrupt/lost hoy!
     *
     *  ErrorRate = 0.00001
     *    → Protita 100,000 packet er moddhe ~1 ta DROP hobe
     *    → Khub kom rate — but enough TCP er behavior test korte!
     *
     *  KENO error model lagacchhi?
     *    → Real network e packet loss hoy (interference, congestion)
     *    → Packet loss hole TCP er cwnd DRASTICALLY kome!
     *    → Error model CHARA shob packet thik pouchchhabe
     *      → cwnd shudu uthbe, nambe na → BORING! Sawtooth dekhte pabo na!
     *    → Error model DIYE realistic behavior test kori
     *      → cwnd rise, DROP, recover → SAWTOOTH pattern! 📈📉📈
     *
     *  Analogy: Courier service test kortechhish.
     *    Deliberately bolchho "100,000 parcel er moddhe 1 ta HAARIYE dao."
     *    Tahole dekhbi courier company (TCP) kivabe react kore —
     *    slow down kore ki na, retry kore ki na.
     */
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(0.00001));

    // n1 (receiver) er NIC te error model lagao
    // devices.Get(1) = n1 er NIC
    devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));


    /* ===============================================================
     *  BLOCK 7: INTERNET STACK + IP ADDRESS
     * ===============================================================
     *  ⚠️ Subtle difference: Subnet mask = 255.255.255.252 (/30)!
     *
     *  first.cc te chilo /24 (255.255.255.0) = 254 hosts
     *  Ekhane /30 = SHUDU 2 ta usable host IP!
     *
     *    /30 subnet:
     *      .0 = network address (assign hoy na)
     *      .1 = n0 (host 1)
     *      .2 = n1 (host 2)
     *      .3 = broadcast (assign hoy na)
     *    Just 2 ta usable — exactly amader 2 node er jonno PERFECT!
     *
     *  Analogy: /24 = 254 flat er apartment building — onek jagah waste
     *           /30 = 2 flat er choto building — exact fit!
     */
    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.252");  // /30 — shudu 2 host!
    Ipv4InterfaceContainer interfaces = address.Assign(devices);
    // n0 → 10.1.1.1
    // n1 → 10.1.1.2


    /* ===============================================================
     *  BLOCK 8: RECEIVER APPLICATION — PacketSink 🗑️ — NOTUN!
     * ===============================================================
     *  first.cc te UdpEchoServer chilo — ekhane PacketSink!
     *
     *  PacketSink ki?
     *    Ja ashbe shudu RECEIVE kore KHAIYE FELBE!
     *    Echo kore fire pathabe NA!
     *
     *  ──────────────────────────────────────────
     *  Feature    │ UdpEchoServer    │ PacketSink
     *  ──────────────────────────────────────────
     *  Receive?   │ Haan             │ Haan
     *  Reply?     │ HAA! Echo kore   │ NA! Chup thake
     *  Protocol   │ UDP              │ TCP
     *  Analogy    │ Pahar (echo)     │ Dustbin 🗑️
     *  ──────────────────────────────────────────
     *
     *  TcpSocketFactory → TCP use korbe (first.cc te UDP chilo!)
     *
     *  Ipv4Address::GetAny() → "Jokono IP theke ashuk — accept!"
     *    (0.0.0.0 er moto)
     *
     *  sinkPort = 8080 → Port 8080 e listen korbe
     *
     *  Start = 0s! (first.cc te server 1s e start hoto)
     *    Keno 0s? TCP connection setup e time lage (3-way handshake: SYN→SYN-ACK→ACK)
     *    Tai receiver ke AGE ready thakte hobe!
     */
    uint16_t sinkPort = 8080;

    // n1 er IP (10.1.1.2) + port 8080 = destination address
    Address sinkAddress(InetSocketAddress(interfaces.GetAddress(1), sinkPort));

    // PacketSink banao — TCP te, port 8080 e listen korbe
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory",
                                      InetSocketAddress(Ipv4Address::GetAny(), sinkPort));

    // n1 te install koro
    ApplicationContainer sinkApps = packetSinkHelper.Install(nodes.Get(1));
    sinkApps.Start(Seconds(0.));   // 0s e start — receiver AAGE ready!
    sinkApps.Stop(Seconds(20.));   // 20s e stop


    /* ===============================================================
     *  BLOCK 9: TCP SOCKET + TRACE CONNECT — PURA CODE ER CORE! 🔑
     * ===============================================================
     *
     *  ═══════════════════════════════════════════════════════
     *  ETA holo fourth.cc er concept REAL NETWORK e apply!
     *  fourth.cc te simple integer trace korechilam.
     *  Ekhane ACTUAL TCP er cwnd trace kortechhi!
     *  ═══════════════════════════════════════════════════════
     */

    // ── LINE 1: TCP socket MANUALLY create koro ──
    //
    // first.cc te UdpEchoClient helper nijei socket banato internally.
    // Ekhane NIJEI banacchhi — karon socket er upor trace connect korte hobe!
    //
    // Keno manually? (Code er comment e bola ache):
    //   1. OnOff app er socket App Start AAGE create hoy na
    //      → tai aage theke trace connect korte pari na
    //   2. OnOff app er socket PRIVATE — access korte pari na
    //   3. Tai nijer socket baniye nijei control kori
    //
    // nodes.Get(0) = n0 te socket banao
    // TcpSocketFactory = TCP type er socket
    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(nodes.Get(0),
                                                     TcpSocketFactory::GetTypeId());

    // ── LINE 2: cwnd er upor TRACE CONNECT! ──
    //
    // ★ fourth.cc er concept EKHANE apply hocche! ★
    //
    // fourth.cc te:
    //   myObject->TraceConnectWithoutContext("MyInteger", MakeCallback(&IntTrace));
    //   → simple integer change hole IntTrace call hotoh
    //
    // Ekhane:
    //   ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));
    //   → TCP er cwnd change hole CwndChange call hobe!
    //
    // "CongestionWindow" = TCP socket er cwnd holo TracedValue
    // CwndChange = cwnd change hole ei function call hobe
    //
    // cwnd jokhon 1→2, 2→4, 4→8 (slow start)
    //   or 8→4 (loss er por) — PROTIBARI CwndChange call hobe!
    //
    // Analogy: fourth.cc te alarm lagaiyechilam locker e.
    //   Ekhane alarm lagachhish TCP er SPEEDOMETER e!
    //   Speed (cwnd) barey ba kome — notification ashey!
    ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow",
                                             MakeCallback(&CwndChange));


    /* ===============================================================
     *  BLOCK 10: SENDER APPLICATION — TutorialApp 📤 — NOTUN!
     * ===============================================================
     *  first.cc te UdpEchoClient chilo — 1 packet, UDP, echo.
     *  Ekhane TutorialApp — 1000 packets, TCP, one-way (no echo)!
     *
     *  Keno custom app? Keno UdpEchoClient/OnOff use korini?
     *    → UdpEchoClient = UDP (cwnd concept NAI UDP te!)
     *    → OnOff app er socket private + late create hoy
     *    → Tai nijer TutorialApp — socket ke nijei control kori
     *
     *  Setup parameters:
     *  ──────────────────────────────────────────
     *  Parameter       │ Value     │ Meaning
     *  ──────────────────────────────────────────
     *  ns3TcpSocket    │ socket    │ EI socket diye data jabe
     *                  │           │ (trace connected ache!)
     *  sinkAddress     │ n1:8080   │ Destination
     *  1040            │ bytes     │ Protita packet size
     *  1000            │ count     │ Total 1000 ta packet
     *  DataRate("1Mbps")│ rate     │ 1 Mbps e pathabe
     *  ──────────────────────────────────────────
     */
    Ptr<TutorialApp> app = CreateObject<TutorialApp>();
    app->Setup(ns3TcpSocket,    // amader banano socket (cwnd trace connected!)
               sinkAddress,      // destination: n1, port 8080
               1040,             // packet size: 1040 bytes
               1000,             // total packets: 1000 ta
               DataRate("1Mbps")); // sending rate: 1 Megabit per second

    // n0 te app install koro
    nodes.Get(0)->AddApplication(app);

    // 1s e start, 20s e stop
    // 1s theke 20s porjonto continuously data pathabe
    app->SetStartTime(Seconds(1.));
    app->SetStopTime(Seconds(20.));


    /* ===============================================================
     *  BLOCK 11: PACKET DROP TRACE — 2nd TRACE! 📉
     * ===============================================================
     *  Arekta trace connect! (Code e total 2 ta trace ache)
     *
     *  n1 er NIC er "PhyRxDrop" trace source er sathe
     *  RxDrop callback connect kortechhi.
     *
     *  Mane: n1 er NIC e packet DROP hole (error model er karone)
     *        → RxDrop() AUTOMATICALLY call hobe
     *        → "RxDrop at 3.72501" print hobe
     *
     *  2 ta TRACE er summary:
     *  ──────────────────────────────────────────────────
     *  Trace             │ Kothay      │ Ki track kore      │ Callback
     *  ──────────────────────────────────────────────────
     *  CongestionWindow  │ TCP socket  │ cwnd value change  │ CwndChange()
     *                    │ (n0)        │                    │
     *  PhyRxDrop         │ NIC (n1)    │ Packet drop event  │ RxDrop()
     *  ──────────────────────────────────────────────────
     *
     *  Analogy:
     *    CwndChange = Speedometer alarm — speed change hole notification
     *    RxDrop = Parcel haariye gele notification
     */
    devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeCallback(&RxDrop));


    /* ===============================================================
     *  BLOCK 12: RUN SIMULATION 🚀
     * ===============================================================
     *
     *  FULL EXECUTION FLOW:
     *  ─────────────────────────────────────────────────────────
     *  Time 0s:    PacketSink START (n1, port 8080, TCP)
     *              "Ami ready! Data ashuk!"
     *
     *  Time 1s:    TutorialApp START (n0)
     *              TCP 3-way handshake: SYN → SYN-ACK → ACK
     *              cwnd = 1 packet (536 bytes) — SLOW START!
     *              CwndChange callback fire hocche protibari!
     *
     *  Time 1-20s: n0 CONTINUOUSLY data pathacche n1 ke
     *              cwnd aste aste baRchhe (1→2→4→8→16...)
     *              Terminal e protita cwnd change print hocche
     *
     *  Random:     Error model PACKET DROP kore! 💥
     *              "RxDrop at 3.72" print hoy
     *              TCP detect kore "loss hoyeche!"
     *              cwnd HALF hoye jay! (NewReno behavior)
     *              CwndChange: "3.72  7500" print hoy
     *
     *              TCP recover kore — cwnd abar baRe
     *              Abar loss → abar drop → abar recover...
     *              ═══ SAWTOOTH pattern! ═══
     *
     *  Time 20s:   Simulator::Stop() — shob bondho
     *  ─────────────────────────────────────────────────────────
     *
     *  EXPECTED OUTPUT (terminal e):
     *  ─────────────────────────────────────────────────────────
     *  1.00037     536          ← cwnd start (1 packet = 536 bytes)
     *  1.0079      1072         ← cwnd baRchhe! Slow Start!
     *  1.01543     1608         ← baRchhe!
     *  1.02296     2144         ← baRchhe!
     *  ...
     *  1.5         15000        ← onek baRe geche!
     *  ...
     *  RxDrop at 3.72501        ← PACKET DROP! 💥 Error model!
     *  3.72501     7500         ← cwnd HALF! TCP detected loss!
     *  ...
     *  3.8         7800         ← recovery — aste aste abar baRchhe
     *  3.9         8100
     *  ...
     *  ─────────────────────────────────────────────────────────
     */

    // 20 second e forcefully stop
    // TCP flow continuously cholte pare — tai explicit stop MUST
    Simulator::Stop(Seconds(20));

    Simulator::Run();       // SIMULATION START!
    Simulator::Destroy();   // Memory cleanup

    return 0;
}


/*
 * ===================================================================
 *  BUILD ORDER:
 * ===================================================================
 *  1.  TCP Configuration (Config::SetDefault — NewReno, cwnd=1, ClassicRecovery)
 *  2.  Create nodes (n0, n1)
 *  3.  Install P2P link (5Mbps, 2ms)
 *  4.  Install Error Model on n1 (random packet drop!)     ← NOTUN
 *  5.  Install TCP/IP stack
 *  6.  Assign IP (/30 subnet — shudu 2 host)
 *  7.  Install PacketSink on n1 (TCP receiver, port 8080)  ← NOTUN
 *  8.  Create TCP socket MANUALLY on n0                    ← NOTUN
 *  9.  TRACE CONNECT cwnd → CwndChange callback            ← NOTUN (fourth.cc concept!)
 *  10. Install TutorialApp on n0 (TCP sender, 1000 packets) ← NOTUN
 *  11. TRACE CONNECT PhyRxDrop → RxDrop callback           ← NOTUN
 *  12. Simulator::Stop(20s) + Run
 * ===================================================================
 *
 *  KEY CONCEPTS — je gula fifth.cc NOTUN shekhachchhe:
 *  ═══════════════════════════════════════════════════════
 *
 *  1. TCP (vs UDP):
 *     UDP = fire and forget, no congestion control
 *     TCP = reliable, congestion control (cwnd!), flow control
 *
 *  2. Congestion Window (cwnd):
 *     = koyта packet ekbar e pathate pare bina ACK wait na kore
 *     Slow Start → exponential growth (1,2,4,8,16...)
 *     Loss → cwnd HALF! → aste aste abar baRe
 *     = SAWTOOTH pattern!
 *
 *  3. NewReno Algorithm:
 *     = TCP congestion control algorithm
 *     Slow Start + Congestion Avoidance + Fast Recovery
 *     Linux er CUBIC/BBR er alternative (slides e chilo!)
 *
 *  4. Error Model:
 *     = Deliberately packet drop kore (simulate real network loss)
 *     Chara shob thik pouchchhabe → cwnd shudu uthbe → boring
 *     Diye realistic SAWTOOTH behavior dekhte pai!
 *
 *  5. PacketSink (vs Echo):
 *     Echo = ja ashbe same fire pathabe (protidhwoni)
 *     Sink = ja ashbe khaiye felbe, reply na (dustbin 🗑️)
 *
 *  6. Manual Socket + TraceConnect:
 *     fourth.cc concept REAL network e apply!
 *     Socket nijei banao → cwnd TracedValue e callback connect koro
 *     → cwnd change hole automatically data collect!
 *
 *  7. /30 Subnet:
 *     255.255.255.252 — shudu 2 ta usable host IP
 *     .0=network, .1=host1, .2=host2, .3=broadcast
 *     2 ta node er jonno EXACT fit!
 *  ═══════════════════════════════════════════════════════
 *
 *
 *  EXAM QUESTIONS:
 *  ─────────────────────
 *  Q: fifth.cc te UDP er bodole TCP keno?
 *  A: cwnd (congestion window) SHUDU TCP te ache!
 *     UDP te congestion control er concept-i NAI!
 *     Amra cwnd trace korte chacchhi — tai TCP MUST.
 *
 *  Q: PacketSink ar UdpEchoServer er difference?
 *  A: EchoServer data receive kore ECHO (same data) fire pathay.
 *     PacketSink shudu receive kore — reply na. Dustbin! 🗑️
 *
 *  Q: Error model keno lagano?
 *  A: Real network e packet loss hoy. Error model chara cwnd
 *     shudu baRbe, kono drop hobena — SAWTOOTH dekhte pabo na!
 *     Error model = realistic TCP behavior test.
 *
 *  Q: Config::SetDefault ki kore?
 *  A: Pura simulation er shob TCP socket er default settings
 *     change kore. Jemon shob phone er default ringtone.
 *
 *  Q: Keno custom TutorialApp? OnOff dile hoto na?
 *  A: OnOff app er socket PRIVATE + LATE create hoy.
 *     Trace connect korte pari na! Tai nijer app banano holo
 *     jekhane socket ke nijei control kori.
 *
 *  Q: /30 subnet mask mane ki?
 *  A: 255.255.255.252 — shudu 2 ta usable host IP.
 *     .0=network, .1=n0, .2=n1, .3=broadcast.
 *     2 node er jonno EXACT fit — IP waste hoy na!
 *
 *  Q: NS_LOG_UNCOND ar NS_LOG_INFO er difference?
 *  A: NS_LOG_INFO → LogComponentEnable korle i print hoy
 *     NS_LOG_UNCOND → SHOBSOMOY print hoy, enable na korleo!
 *
 *  Q: cwnd er SAWTOOTH pattern ki?
 *  A: cwnd aste aste baRe (slow start/congestion avoidance)
 *     → packet loss hoy → cwnd HALF kome (NewReno)
 *     → abar aste aste baRe → abar loss → abar kome
 *     → ei ura-naama = SAWTOOTH (করাতের দাঁত) pattern!
 *
 *  Q: Ei code e koyটa trace ache?
 *  A: 2 ta!
 *     1. CongestionWindow → cwnd change track (CwndChange)
 *     2. PhyRxDrop → packet drop track (RxDrop)
 * ===================================================================
 */