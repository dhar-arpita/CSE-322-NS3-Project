/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * ===================================================================
 *  fourth.cc — NS-3 er FOURTH example script
 *  CSE 322: Computer Networks
 * ===================================================================
 *
 *  PURA CODE KI KORCHHE? (Big Picture)
 *  ------------------------------------
 *  Kono network NAI! Kono node nai, P2P nai, WiFi nai, CSMA nai!
 *
 *  Eta shudu ekta concept shekhachchhe:
 *  ══════════════════════════════════════════════
 *  TracedValue — jokhon ekta variable er value CHANGE hoy,
 *  AUTOMATICALLY ekta function call hoye jay!
 *  ══════════════════════════════════════════════
 *
 *  Flow:
 *    m_myInt = 0 (default)
 *        ↓
 *    m_myInt = 1234 (value CHANGE kora holo)
 *        ↓
 *    AUTOMATICALLY print hoy: "Traced 0 to 1234"
 *
 *  Tui MANUALLY kono print statement likhish ni!
 *  Shudu variable er value change korechhi — NS-3 NIJEI detect kore
 *  bolchhe "hey! value change hoyeche! 0 → 1234!"
 *
 *  ══════════════════════════════════════════════
 *  Analogy: SECURITY ALARM SYSTEM! 🚨
 *  ══════════════════════════════════════════════
 *  Bina alarm: Toke nijeke bar bar daroja check korte hobe
 *              "keu dhuklo ki na?"
 *  Alarm lagale: Keu daroja khulle AUTOMATICALLY alarm beje uthbe!
 *                Toke check korte hobe na — system nijei janaibe.
 *
 *  TracedValue = variable er upor alarm lagano!
 *  Callback    = alarm bajle ki hobe (kon function call hobe)
 *  Connect     = alarm er wire lagano (variable + callback jore deoa)
 *
 *
 *  first/second/third vs fourth — SHOMPURNO ALAG!
 *  ─────────────────────────────────────────────────
 *  Feature          │ first/second/third │ fourth.cc
 *  ─────────────────────────────────────────────────
 *  Network ache?    │ Haan!              │ NAI!
 *  Ki shekhachchhe? │ Network simulation │ Tracing system
 *  Nodes ache?      │ Haan               │ Na
 *  Output ki?       │ sent/received      │ "Traced 0 to 1234"
 *  Key concept      │ Node,Link,App      │ TracedValue,Callback
 *  ─────────────────────────────────────────────────
 *
 *
 *  Real simulation e kothay use hoy?
 *  ─────────────────────────────────
 *  TCP er congestion window (cwnd) trace korte paro!
 *  Jokhon cwnd change hobe → tomar function call hobe
 *  → file e log koro → graph banao!
 *
 *  Example:
 *    void CwndChange(uint32_t oldCwnd, uint32_t newCwnd) {
 *        file << time << " " << newCwnd << endl;  // graph data!
 *    }
 *    tcpSocket->TraceConnectWithoutContext("CongestionWindow",
 *                                          MakeCallback(&CwndChange));
 * ===================================================================
 */


/* ===================================================================
 *  BLOCK 1: HEADER FILES (#include)
 * ===================================================================
 *  first/second/third.cc theke SHOMPURNO ALAG headers!
 *  Kono network-module, internet-module, applications-module NAI!
 *  Karon kono network banacchi na — shudu tracing concept shikchhi.
 */

// NS-3 er base Object class — shob NS-3 object etar theke inherit kore
// Tracing system shudu Object er subclass e kaaj kore
#include "ns3/object.h"

// Simulator class — ekhane directly use hocche na, but include ache
#include "ns3/simulator.h"

// Trace source ke accessible korar jonno — AddTraceSource e lagey
#include "ns3/trace-source-accessor.h"

// ★ TracedValue — EI PURA CODE ER HERO! ★
// Variable er upor "alarm" lagay!
// Normal int → change korle KICHHU hoy na
// TracedValue<int> → change korle AUTOMATICALLY callback function call hoy!
//
//   int32_t normalVar;           → Normal daroja (alarm nai)
//   TracedValue<int32_t> m_myInt; → Alarm-wala daroja (change = alarm baje!)
//
// Analogy:
//   Normal variable = normal locker
//   TracedValue     = smart locker — khulle phone e notification ashey!
#include "ns3/traced-value.h"

// Unsigned integer attribute type
#include "ns3/uinteger.h"

// Standard C++ — cout use korar jonno (terminal e print)
#include <iostream>


using namespace ns3;


/* ===================================================================
 *  BLOCK 2: MyObject CLASS — Traced Variable er GHOR 🏠
 * ===================================================================
 *
 *  Ekta nijer class banacchi ja Object theke inherit kore.
 *  Ei class er moddhe ekta TracedValue variable (m_myInt) ache.
 *
 *  Keno Object theke inherit?
 *    NS-3 er tracing system SHUDU Object er subclass e kaaj kore.
 *    Normal C++ class e TracedValue use korte parbi na.
 *    Analogy: Alarm system kaaj korbe shudu jodi device ta
 *    "smart home network" e connected thake (Object theke inherit).
 *    Normal device e alarm lagano jabe na.
 *
 *  Class er vitore 3 ta jinish ache:
 *    1. GetTypeId()  → NS-3 te class REGISTER kore
 *    2. MyObject()   → Constructor (empty)
 *    3. m_myInt      → TracedValue variable (THE STAR! ⭐)
 */

/**
 * Tutorial 4 - a simple Object to show how to hook a trace.
 */
class MyObject : public Object
{
  public:
    /**
     * Register this type.
     * @return The TypeId.
     */

    /* ───────────────────────────────────────────────
     *  GetTypeId() — NS-3 er type system e REGISTER koro
     * ───────────────────────────────────────────────
     *  Eta NS-3 ke bole:
     *    "MyObject naam er class ache, ar etar moddhe
     *     ekta traceable variable ache!"
     *
     *  Analogy: Smart locker kinar por WARRANTY CARD e
     *  register koro — "ei locker e alarm system ache,
     *  model number eta, alarm er naam eta."
     */
    static TypeId GetTypeId()
    {
        static TypeId tid = TypeId("MyObject")

                                // "Amar parent class holo Object"
                                .SetParent<Object>()

                                // Ei class "Tutorial" group er part
                                .SetGroupName("Tutorial")

                                // NS-3 ke bolchho "tumi nijeo ei class er
                                // object banate paro" (CreateObject diye)
                                .AddConstructor<MyObject>()

                                // ★ SHOB CHEYE IMPORTANT LINE! ★
                                // Trace source REGISTER kortechhe:
                                //
                                // "MyInteger"
                                //    → Trace er NAAM (pore ei naam diye connect korbo)
                                //
                                // "An integer value to trace."
                                //    → Human-readable description
                                //
                                // MakeTraceSourceAccessor(&MyObject::m_myInt)
                                //    → KONTA variable ke trace korbi?
                                //      m_myInt variable ke!
                                //
                                // "ns3::TracedValueCallback::Int32"
                                //    → Callback er type — int32 value change hole
                                //      callback e (oldValue, newValue) jabe
                                //
                                // Analogy: "Amar locker e alarm ache.
                                //   Alarm er naam 'MyInteger'.
                                //   m_myInt daroja te lagano.
                                //   Keu khulle (old, new) value report korbe."
                                .AddTraceSource("MyInteger",
                                                "An integer value to trace.",
                                                MakeTraceSourceAccessor(&MyObject::m_myInt),
                                                "ns3::TracedValueCallback::Int32");
        return tid;
    }

    // Constructor — empty, kichhu special hocche na
    // m_myInt er default value automatically 0 hobe (int32_t default)
    MyObject()
    {
    }

    // ★ THE STAR OF THE SHOW! ★
    //
    // TracedValue<int32_t> — eta NORMAL int na!
    // Dekhte same lage, use o korlei same lage.
    // BUT internally MONITOR kortechhe!
    // Value change hole → connected callback function ke AUTO call kore!
    //
    // ┌────────────────────────────────────┐
    // │  Normal int32_t:                   │
    // │    value = 5                       │
    // │    value = 10  → kichhu hoy na     │
    // │                                    │
    // │  TracedValue<int32_t>:             │
    // │    value = 5                       │
    // │    value = 10  → ALARM! 🚨         │
    // │    → callback(5, 10) AUTO call!    │
    // └────────────────────────────────────┘
    TracedValue<int32_t> m_myInt; //!< The traced value.
};


/* ===================================================================
 *  BLOCK 3: CALLBACK FUNCTION — Alarm bajle KI hobe? 🔔
 * ===================================================================
 *
 *  Ei function MANUALLY call korte hoy NA!
 *  TracedValue NIJEI call kore AUTOMATICALLY
 *  jokhon value change hoy!
 *
 *  Parameters (NS-3 automatically bhore dey):
 *    oldValue = AAGER value (alarm bajar AAGE)
 *    newValue = NOTUN value (alarm bajar PORE)
 *
 *  Tui chaile ei function e JETA KHOSHA teta korte paro:
 *    - Print korte paro (ekhane eta hocche)
 *    - File e log korte paro (graph data collect)
 *    - Counter barate paro
 *    - Alert pathate paro
 *    - Kichhu na kore ignore o korte paro
 *
 *  Analogy: Alarm bajle ki hobe?
 *    → Security guard ke phone jabe (print)
 *    → CCTV recording start hobe (log to file)
 *    → Police ke call hobe (alert)
 *    Tomar choice! Ekhane just print kortechhi.
 *
 *  Real simulation e eta diye TCP cwnd data collect kore
 *  graph banate paro!
 */
void
IntTrace(int32_t oldValue, int32_t newValue)
{
    std::cout << "Traced " << oldValue << " to " << newValue << std::endl;
}


/* ===================================================================
 *  BLOCK 4: MAIN FUNCTION — Pura FLOW ekhane! 🚀
 * ===================================================================
 *
 *  EXECUTION FLOW (step by step ki hocche):
 *  ─────────────────────────────────────────────────────
 *
 *  STEP 1: CreateObject<MyObject>()
 *    │ m_myInt = 0 (default)
 *    │ "MyInteger" trace source REGISTERED ✅
 *    │ callback connected? ❌ NA (alarm er wire lagano hoy ni)
 *    │ Analogy: Smart locker kina anlam. Alarm built-in.
 *    │          But electrician eshe wire connect kore ni.
 *    │          Ekhon locker khulle o alarm bajbe na.
 *    │
 *    ▼
 *  STEP 2: TraceConnectWithoutContext("MyInteger", IntTrace)
 *    │ m_myInt ← wire → IntTrace() function
 *    │ callback connected? ✅ HAA!
 *    │ m_myInt still = 0 — kono output NAI ekhono
 *    │ Analogy: Electrician wire connect korlo.
 *    │          Ekhon locker khulle alarm bajbe!
 *    │          But ekhono keu kholeni.
 *    │
 *    ▼
 *  STEP 3: m_myInt = 1234
 *    │ TracedValue detect kore: old=0, new=1234
 *    │ "0 ≠ 1234 → VALUE CHANGE HOYECHE!"
 *    │ IntTrace(0, 1234) AUTOMATICALLY call!
 *    │ cout << "Traced 0 to 1234"
 *    │ Analogy: Keu locker khullo! 🚨
 *    │          Alarm bajlo! Security guard ke notification gelo!
 *    │
 *    ▼
 *  OUTPUT: "Traced 0 to 1234"
 *    │
 *    ▼
 *  return 0 → Program END
 *  ─────────────────────────────────────────────────────
 */
int
main(int argc, char* argv[])
{
    /* ───────────────────────────────────────────────
     *  STEP 1: Object CREATE koro
     * ───────────────────────────────────────────────
     *  Ptr<MyObject> = NS-3 er smart pointer (memory auto-manage)
     *  CreateObject<MyObject>() = object banao
     *
     *  Internally ki hocche:
     *    → MyObject() constructor call hoy (empty — kichhu kore na)
     *    → m_myInt automatically create hoy, default value = 0
     *    → GetTypeId() internally call hoy
     *      → "MyInteger" naam e m_myInt ke trace source hisebe REGISTER
     *      → NS-3 ekhon JANE "ei object e traceable variable ache"
     *
     *  Ekhon state:
     *    myObject:
     *    ├── m_myInt = 0 (default)
     *    ├── Trace source "MyInteger" REGISTERED ✅
     *    └── Callback CONNECTED na ❌
     *        (alarm ache, but wire lagano hoy ni)
     *
     *  Analogy: Smart locker kina anlam.
     *           Alarm system built-in ache.
     *           But electrician call kore wire CONNECT korini.
     *           Ekhon locker khulle o alarm bajbe na.
     */
    Ptr<MyObject> myObject = CreateObject<MyObject>();


    /* ───────────────────────────────────────────────
     *  STEP 2: Callback CONNECT koro — WIRE LAGAO! 🔌
     * ───────────────────────────────────────────────
     *  ★ SHOB CHEYE CRITICAL LINE! ★
     *
     *  Ki kortechhe:
     *    "MyInteger" → konta trace source? → m_myInt
     *    MakeCallback(&IntTrace) → alarm bajle konta function call hobe?
     *                            → IntTrace() function!
     *
     *  Mane: "MyInteger" er sathe IntTrace CONNECT koro.
     *        Jokhon o m_myInt er value change hobe
     *        → IntTrace(old, new) AUTOMATICALLY call hobe!
     *
     *  "WithoutContext" mane ki?
     *    Context = extra info (like "konta node theke ashchhe")
     *    Ekhane context pass kortechhi na — simple version.
     *    TraceConnectWithContext() diye context o pathate paro.
     *
     *  Ekhon state:
     *    myObject:
     *    ├── m_myInt = 0 (STILL same — value change hoy ni)
     *    ├── Trace source "MyInteger" REGISTERED ✅
     *    └── Callback CONNECTED ✅ → IntTrace()
     *
     *    m_myInt ──── wire ────→ IntTrace()
     *    (variable)               (alarm function)
     *
     *  ⚠️ EKHONO kono output NAI terminal e!
     *     Karon m_myInt er value ekhono change hoy ni — 0-i ache.
     *
     *  ⚠️ EXAM TRAP:
     *     Ei line na likhle → alarm connected na
     *     → value change korleo IntTrace call hobena
     *     → terminal e KONO output hobena! Blank screen!
     *
     *  Analogy: Electrician eshe alarm er wire CONNECT korlo.
     *           Ekhon locker khulle → alarm bajbe!
     *           But ekhono keu kholeni — tai ekhono silence.
     */
    myObject->TraceConnectWithoutContext("MyInteger", MakeCallback(&IntTrace));


    /* ───────────────────────────────────────────────
     *  STEP 3: VALUE CHANGE — ALARM TRIGGER! 🚨
     * ───────────────────────────────────────────────
     *  m_myInt er value 0 theke 1234 e change korchhi.
     *
     *  BUT! m_myInt holo TracedValue — normal int na!
     *  TracedValue er overloaded = operator internally kaam kore:
     *
     *    m_myInt = 1234;
     *        ↓
     *    TracedValue detect kore:
     *        old = 0, new = 1234
     *        "0 ≠ 1234 → VALUE CHANGE HOYECHE!"
     *        ↓
     *    Connected callback ache? → HAA! IntTrace ache!
     *        ↓
     *    IntTrace(0, 1234) AUTOMATICALLY call hoy!
     *        ↓
     *    IntTrace execute kore:
     *        cout << "Traced " << 0 << " to " << 1234
     *        ↓
     *    TERMINAL OUTPUT: "Traced 0 to 1234"
     *
     *
     *  ⚠️ TUI MANUALLY kono print statement LIKHISH NI!
     *     Shudu value change korechho — baki shob AUTOMATIC!
     *
     *
     *  ⚠️ EDGE CASES (exam e ashte pare):
     *
     *  Case 1: Jodi SAME value assign kortam?
     *    m_myInt = 0;  // already 0!
     *    → TracedValue check: old=0, new=0
     *    → "0 == 0 → SAME! Change hoy ni!"
     *    → Callback call hoy NA!
     *    → KONO output NAI!
     *
     *  Case 2: Jodi DUIBAR value change kortam?
     *    m_myInt = 1234;  // 0 → 1234
     *    m_myInt = 5678;  // 1234 → 5678
     *    → Output:
     *       Traced 0 to 1234
     *       Traced 1234 to 5678
     *    → Protibari change = protibari callback!
     *
     *  Case 3: Jodi Connect na kortam? (Step 2 skip)
     *    m_myInt = 1234;
     *    → TracedValue detect kore: change hoyeche!
     *    → Connected callback ache? → ❌ NAI!
     *    → Kichhu hoy na. BLANK SCREEN.
     *
     *
     *  Analogy: Keu locker khullo! 🚨
     *    → Alarm bajlo!
     *    → Security guard er phone e notification gelo!
     *    → Guard dekhchhe: "Locker #MyInteger: 0 theke 1234 e change!"
     */
    myObject->m_myInt = 1234;

    // OUTPUT: "Traced 0 to 1234"

    return 0;  // Program shesh
}


/*
 * ===================================================================
 *  FULL EXECUTION FLOW — Summary
 * ===================================================================
 *
 *  main() start
 *      │
 *      ▼
 *  CreateObject<MyObject>()
 *      │ m_myInt = 0 (default)
 *      │ "MyInteger" REGISTERED ✅
 *      │ callback connected? ❌
 *      │
 *      ▼
 *  TraceConnectWithoutContext("MyInteger", IntTrace)
 *      │ m_myInt ← wire → IntTrace()
 *      │ callback connected? ✅
 *      │ m_myInt still = 0, output NAI
 *      │
 *      ▼
 *  m_myInt = 1234
 *      │ TracedValue: 0 ≠ 1234 → CHANGE!
 *      │ IntTrace(0, 1234) AUTO call!
 *      │
 *      ▼
 *  IntTrace(0, 1234)
 *      │ cout << "Traced 0 to 1234"
 *      │
 *      ▼
 *  TERMINAL: "Traced 0 to 1234"
 *      │
 *      ▼
 *  return 0 → END
 *
 * ===================================================================
 *
 *  KEY COMPONENTS — 4 ta step
 *  ──────────────────────────────────────────────────
 *  Step │ Ki hoy              │ Analogy
 *  ──────────────────────────────────────────────────
 *  1    │ Create Object       │ Smart locker kena
 *       │ TracedValue banay   │ Alarm built-in
 *       │ value = 0           │ but wire lagano hoy ni
 *  ──────────────────────────────────────────────────
 *  2    │ Register (GetTypeId)│ Warranty card e
 *       │ Trace source naam   │ alarm er info register
 *       │ "MyInteger"         │
 *  ──────────────────────────────────────────────────
 *  3    │ Connect callback    │ Electrician wire lagay
 *       │ variable → function │ alarm ← wire → guard
 *  ──────────────────────────────────────────────────
 *  4    │ Value change        │ Keu locker khullo!
 *       │ Callback AUTO call  │ Alarm baje! Guard ke
 *       │ Output print!       │ notification jay!
 *  ──────────────────────────────────────────────────
 *
 *  ⚠️ Step 3 (Connect) chara Step 4 e KICHHU hobe na!
 *     Wire na lagale alarm bajbe na!
 *
 *
 *  EXAM QUESTIONS:
 *  ─────────────────────
 *  Q: TracedValue ar normal variable er difference ki?
 *  A: Normal variable change korle kichhu hoy na.
 *     TracedValue change korle AUTOMATICALLY connected
 *     callback function call hoy. Alarm system er moto!
 *
 *  Q: TraceConnectWithoutContext na korle ki hobe?
 *  A: Value change korte parbi — but kono callback call
 *     hobe na. Terminal e kono output hobena. Blank!
 *     Karon alarm er wire connected na.
 *
 *  Q: IntTrace function ke manually call korte hoy?
 *  A: NA! TracedValue NIJEI call kore automatically.
 *     Tui shudu value change koro — baki automatic.
 *
 *  Q: Same value assign korle (m_myInt = 0 jokhon already 0)?
 *  A: Callback call hoy NA! TracedValue detect kore
 *     "value change hoy ni" → ignore kore.
 *
 *  Q: AddTraceSource e ki korchhe?
 *  A: NS-3 er type system e register kortechhe
 *     "ei class er ei variable ta traceable."
 *     Pore TraceConnect diye connect korte parbi.
 *
 *  Q: fourth.cc er purpose ki?
 *  A: NS-3 er tracing mechanism shekhanor jonno.
 *     Kivabe variable ke monitor kore automatic callback
 *     pao jokhon value change hoy. Real simulation e
 *     TCP cwnd, packet count, delay etc. trace korte
 *     ei concept use hoy.
 *
 *  Q: "WithoutContext" mane ki?
 *  A: Context = extra info like "konta node theke ashchhe."
 *     WithoutContext = context chara simple version.
 *     TraceConnectWithContext diye context o pathate paro.
 * ===================================================================
 */