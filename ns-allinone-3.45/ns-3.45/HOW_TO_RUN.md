# How to Run Black Hole Attack Simulation

## Quick Start Guide

### Step 1: Navigate to NS-3 Directory
```bash
cd /home/arpita/Desktop/Term_3-2/CSE-322-Computer-Networks-Sessional/NS3_Project/ns-allinone-3.45/ns-3.45
```

### Step 2: Choose Your Running Method

## Method 1: Single Quick Test (Fastest)

### Run WITHOUT Black Hole Attack (Baseline):
```bash
./ns3 run "aodv-blackhole-test --nodes=10 --duration=20 --enableBlackHole=false --output=baseline"
```

### Run WITH Black Hole Attack:
```bash
./ns3 run "aodv-blackhole-test --nodes=10 --duration=20 --enableBlackHole=true --maliciousNode=5 --output=attack"
```

**Parameters Explained:**
- `--nodes=10` - Number of nodes in the network
- `--duration=20` - Simulation time in seconds (use 40 for full test as per paper)
- `--enableBlackHole=true/false` - Enable or disable Black Hole attack
- `--maliciousNode=5` - Which node should be malicious (node ID)
- `--output=prefix` - Prefix for output files

---

## Method 2: Run Full Experiment Suite (Recommended for Paper Results)

This runs all experiments automatically (10, 20, 30, 40, 50 nodes):

```bash
# Make script executable (only needed once)
chmod +x run-blackhole-experiments.sh

# Run all experiments
./run-blackhole-experiments.sh
```

**What this does:**
- Runs simulations for 10, 20, 30, 40, 50 nodes
- Each configuration tested WITH and WITHOUT Black Hole attack
- Results saved to `blackhole_results/` directory
- Creates summary CSV file

**Time estimate:** ~10-15 minutes for all experiments

---

## Method 3: Custom Configuration

You can customize any parameter:

```bash
# Example: 30 nodes, 40 seconds, malicious node 15
./ns3 run "aodv-blackhole-test --nodes=30 --duration=40 --enableBlackHole=true --maliciousNode=15 --output=custom_test"
```

---

## Viewing Results

### During Simulation:
You'll see output like:
```
========== Simulation Results ==========
Configuration:
  Nodes: 10
  Duration: 20 s
  Black Hole Attack: ENABLED
  Malicious Node: 5

Performance Metrics:
  Packets Sent: 720
  Packets Received: 150
  Packet Delivery Ratio (PDR): 20.83 %
  Average Throughput: 24.5 kbps
  Average End-to-End Delay: 45.2 ms
========================================
```

### After Full Experiment Suite:
Results are saved in `blackhole_results/`:
- `summary_results.csv` - All results in CSV format
- Individual result files for each test
- FlowMonitor XML files for detailed analysis

---

## Analyzing Results

After running experiments, analyze and plot:

```bash
python3 analyze-blackhole-results.py
```

This will:
- Parse all results from CSV
- Generate comparison plots (PDR, Throughput, Delay vs. Nodes)
- Show comparison table
- Save plots as PNG files

---

## Debugging / Verbose Output

To see detailed AODV behavior and malicious node actions:

```bash
NS_LOG="AodvRoutingProtocol=level_debug" ./ns3 run "aodv-blackhole-test --nodes=10 --enableBlackHole=true --maliciousNode=5"
```

Look for messages like:
- `MALICIOUS NODE: Sending fake RREP for destination...`
- `MALICIOUS NODE: Dropping packet from X to Y`

---

## Example Workflow

### Complete Analysis Workflow:

```bash
# 1. Navigate to directory
cd /home/arpita/Desktop/Term_3-2/CSE-322-Computer-Networks-Sessional/NS3_Project/ns-allinone-3.45/ns-3.45

# 2. Run quick test to verify it works
./ns3 run "aodv-blackhole-test --nodes=10 --duration=20 --enableBlackHole=false"

# 3. Run with attack to see difference
./ns3 run "aodv-blackhole-test --nodes=10 --duration=20 --enableBlackHole=true --maliciousNode=5"

# 4. Run full experiment suite (for paper results)
./run-blackhole-experiments.sh

# 5. Analyze and plot results
python3 analyze-blackhole-results.py
```

---

## Troubleshooting

### If simulation doesn't run:
```bash
# Rebuild everything
./ns3 clean
./ns3 build
```

### If script not executable:
```bash
chmod +x run-blackhole-experiments.sh
```

### If Python script fails:
```bash
# Install matplotlib if needed
pip3 install matplotlib
```

---

## Expected Behavior

### Without Black Hole Attack:
- Higher PDR (packet delivery ratio)
- Better throughput
- Lower delay

### With Black Hole Attack:
- **Significantly lower PDR** (many packets dropped)
- **Reduced throughput** (less data delivered)
- **Increased delay** (packets take longer routes or get lost)

The malicious node will:
1. Send fake route replies claiming it has the best route
2. Attract traffic to itself
3. Drop all packets instead of forwarding them
4. Create a "black hole" where packets disappear

---

## Quick Reference

| Command | Purpose |
|---------|---------|
| `./ns3 run "aodv-blackhole-test --nodes=10 --enableBlackHole=false"` | Quick baseline test |
| `./ns3 run "aodv-blackhole-test --nodes=10 --enableBlackHole=true --maliciousNode=5"` | Quick attack test |
| `./run-blackhole-experiments.sh` | Full experiment suite |
| `python3 analyze-blackhole-results.py` | Analyze and plot results |
| `NS_LOG="AodvRoutingProtocol=level_debug" ./ns3 run ...` | Debug mode |
