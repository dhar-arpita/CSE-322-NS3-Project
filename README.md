# AODV Attack & Defense Simulation in NS-3.45

## CSE 322: Computer Networks Sessional — NS-3 Project
**Student ID:** 2105137  
**Base Paper:** Gurung & Chauhan (2017) — *"Performance analysis of black-hole attack mitigation protocols under gray-hole attacks in MANET"*, Wireless Networks, vol. 25, pp. 975–988.

---

## Project Overview

This project modifies the AODV routing protocol in NS-3.45 to simulate **4 types of attacks** and **3 defense mechanisms** on Mobile Ad-Hoc Networks (MANETs). The Trust-Based AODV defense is our **original contribution** beyond the paper.

| Feature | Type | Key Behavior |
|---------|------|-------------|
| Blackhole | Attack | Fake RREP (SeqNo+1000) + Drop ALL packets |
| Grayhole | Attack | Fake RREP (SeqNo+1000) + Drop 50% packets |
| Smart Blackhole | Attack | No fake RREP + Drop ALL packets |
| Smart Grayhole | Attack | No fake RREP + Drop 50% packets |
| IDS-AODV | Defense | Ignores first RREP (assumed from attacker) |
| MBDP-AODV | Defense | Statistical analysis of SeqNo (Mean + StdDev) |
| Trust-Based AODV | Defense (Novel) | Monitors actual data forwarding via promiscuous mode |

### Detection Capability

| Attack | IDS-AODV | MBDP-AODV | Trust-Based |
|--------|----------|-----------|-------------|
| Blackhole | Detects | Detects | Detects |
| Grayhole | Detects | Detects | Detects |
| Smart Blackhole | **FAILS** | **FAILS** | Detects |
| Smart Grayhole | **FAILS** | **FAILS** | Detects |

**Key Insight:** IDS and MBDP work at the route discovery layer (RREP analysis). Trust works at the data forwarding layer (monitoring actual packet delivery). Smart attacks behave normally at the route layer — so only Trust can detect them.



---

## Setup & Installation

### Prerequisites
- NS-3.45 (ns-allinone-3.45)
- Python 3 with `pandas` and `matplotlib`
- LaTeX distribution (for report compilation)

### Step 1: Copy modified AODV files
```bash
cp src/aodv-routing-protocol.h  ~/ns-allinone-3.45/ns-3.45/src/aodv/model/
cp src/aodv-routing-protocol.cc ~/ns-allinone-3.45/ns-3.45/src/aodv/model/
```

### Step 2: Copy simulation scripts
```bash
cp scratch/aodv_bh_gh.cc  ~/ns-allinone-3.45/ns-3.45/scratch/
cp scratch/zigbee_sim.cc   ~/ns-allinone-3.45/ns-3.45/scratch/
```

### Step 3: Build
```bash
cd ~/ns-allinone-3.45/ns-3.45
./ns3 build
```

---

## Running Simulations

### Paper-Based Experiments (802.11, varying speed)

```bash
# Normal AODV
./ns3 run "scratch/aodv_bh_gh --nNodes=50 --attack=none --speed=5"

# Grayhole attack, no defense
./ns3 run "scratch/aodv_bh_gh --nNodes=50 --attack=grayhole --nMalicious=1 --speed=5"

# Grayhole + Trust defense
./ns3 run "scratch/aodv_bh_gh --nNodes=50 --attack=grayhole --defense=trust --speed=5"

# Smart Grayhole + Trust
./ns3 run "scratch/aodv_bh_gh --nNodes=50 --attack=smartgray --defense=trust --speed=15"

# Blackhole + MBDP
./ns3 run "scratch/aodv_bh_gh --nNodes=50 --attack=blackhole --defense=mbdp --speed=25"
```

### Checklist Experiments (802.11 static, varying parameters)

```bash
# Single run example
./ns3 run "scratch/aodv_bh_gh --nNodes=40 --nFlows=20 --pps=200 --areaMult=3 --scenario=grayhole_trust --simTime=100"

# Automated (all 180 runs, parallel in 4 terminals)
bash run_all_wifi.sh 1   # Vary Nodes (45 runs)
bash run_all_wifi.sh 2   # Vary Flows (45 runs)
bash run_all_wifi.sh 3   # Vary PPS (45 runs)
bash run_all_wifi.sh 4   # Vary Area (45 runs)

# Or all at once
bash run_all_wifi.sh all
```

### 802.15.4 Experiments

```bash
# Single run
./ns3 run "scratch/zigbee_sim --nNodes=20 --nFlows=5 --pps=100 --areaMult=3 --scenario=normal"

# Automated
bash run_all_zigbee.sh all
```

### Available Scenarios
| Scenario String | Attack | Defense |
|----------------|--------|---------|
| `normal` | none | none |
| `grayhole_none` | grayhole | none |
| `grayhole_ids` | grayhole | IDS-AODV |
| `grayhole_mbdp` | grayhole | MBDP-AODV |
| `grayhole_trust` | grayhole | Trust-Based |
| `smartgray_none` | smart grayhole | none |
| `smartgray_ids` | smart grayhole | IDS-AODV |
| `smartgray_mbdp` | smart grayhole | MBDP-AODV |
| `smartgray_trust` | smart grayhole | Trust-Based |

---

## Generating Graphs

### Paper results (X-axis: speed, 4 graphs)
```bash
# Requires: results_v1.csv in current directory
python3 plot_paper_results.py
# Output: graphs_paper/
```

### Checklist results (X-axis: nodes/flows/pps/area, 10 graphs)
```bash
# Requires: results_wifi_*.csv in current directory
python3 plot_graphs.py
# Output: graphs_wifi/
```

---

## Code Modifications Summary

All modifications are in `aodv-routing-protocol.h` and `aodv-routing-protocol.cc`:

| # | Function | Line | What |
|---|----------|------|------|
| 1 | Constructor | 164 | All flags initialized (false/defaults) |
| 2 | GetTypeId | 340 | All attributes registered for NS-3 |
| 3 | Forwarding() | 769 | Blackhole + Smart BH: packet drop |
| 4 | Forwarding() | 777 | Grayhole: probabilistic drop |
| 5 | Forwarding() | 793 | Smart Grayhole: probabilistic drop |
| 6 | Forwarding() | 808 | Trust: expired entry cleanup |
| 7 | Forwarding() | 838 | Trust: check next hop reliability |
| 8 | Forwarding() | 878 | Trust: track packet after send |
| 9 | RouteOutput() | 512 | Trust: expired entry cleanup (source) |
| 10 | RouteOutput() | 541 | Trust: source trust check + tracking |
| 11 | OnOverheardPacket() | 918 | Trust: promiscuous overhear + match |
| 12 | RefreshNeighborRating() | 976 | Trust: EMA formula (0.3*ratio + 0.7*old) |
| 13 | NotifyInterfaceUp() | 1078 | Trust: register promisc callback |
| 14 | RecvRequest() | 1751 | Blackhole: fake RREP SeqNo+1000 |
| 15 | RecvRequest() | 1799 | Grayhole: fake RREP SeqNo+1000 |
| 16 | RecvReply() | 2065 | IDS: ignore first RREP |
| 17 | RecvReply() | 2088 | MBDP: blacklist/suspect check |
| 18 | RecvReply() | 2174 | MBDP: K-reply statistical analysis |
| 19 | RecvReply() | 2276 | MBDP: safe SendPacketFromQueue fix |

---

## Simulation Parameters

### Paper Parameters (Table 4)
| Parameter | Value |
|-----------|-------|
| Nodes | 50 |
| Area | 750m × 750m |
| Simulation Time | 500s |
| Traffic | CBR-UDP, 10kbps |
| Packet Size | 512 bytes |
| Connections | 2 |
| MAC | IEEE 802.11b |
| Speed | 5, 15, 25, 35 m/s |
| Malicious Nodes | 1 or 2 |

### Checklist Parameters (802.11 Static)
| Parameter | Values |
|-----------|--------|
| Nodes | 20, 40, 60, 80, 100 |
| Flows | 10, 20, 30, 40, 50 |
| Packets/sec | 100, 200, 300, 400, 500 |
| Coverage Area | 1×, 2×, 3×, 4×, 5× Tx_range |
| Tx Range | 250m |
| Simulation Time | 100s |

### Metrics Measured
1. Packet Delivery Ratio (PDR)
2. Network Throughput (Kbps)
3. Routing Overhead / NRL
4. End-to-End Delay (ms)
5. Packet Drop Ratio (%)
6. Energy Consumption (J)

---

## Trust-Based AODV — How It Works

1. **Registration:** Node enables promiscuous mode on WiFi interface
2. **Tracking:** When sending a packet via next hop B, creates a pending entry with 1-second expiry
3. **Overhearing:** If B forwards the packet, the promiscuous callback detects it → `heardCount[B]++`
4. **Expiry:** If B drops the packet, entry expires → trust decreases
5. **Trust Score:** `newTrust = 0.3 × (heard/sent) + 0.7 × oldTrust`
6. **Action:** If `trust[B] < 0.5`, route via B is deleted, new route discovered avoiding B

### Trust Decay Example (Malicious Node)
| Packet | Sent | Heard | Ratio | Trust | Status |
|--------|------|-------|-------|-------|--------|
| 1 | 1 | 1 | 1.00 | 1.000 | Trusted |
| 2 | 2 | 1 | 0.50 | 0.850 | Trusted |
| 3 | 3 | 1 | 0.33 | 0.695 | Trusted |
| 4 | 4 | 1 | 0.25 | 0.562 | Trusted |
| 5 | 5 | 1 | 0.20 | 0.452 | **UNTRUSTED** |

---



## Key Findings

1. **Grayhole attack** drops PDR to 42–67%. MBDP and Trust recover to 75–94%.
2. **Smart Grayhole** has lower impact (80–91% PDR) but is undetectable by IDS/MBDP.
3. **Trust-Based defense** is the **only** mechanism that detects smart attacks.
4. Trust has higher routing overhead due to route re-discovery and promiscuous mode.
5. Increasing coverage area beyond 3× Tx_range causes sharp performance degradation.
