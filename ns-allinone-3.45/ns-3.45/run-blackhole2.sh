#!/bin/bash
# ═══════════════════════════════════════════════════
#  Run BH=0,1,2,3 for a given node count + speed
#
#  Usage: bash run-blackhole.sh [nNodes] [speed]
#  Example: bash run-blackhole.sh 20 10
#           bash run-blackhole.sh 40 0
# ═══════════════════════════════════════════════════

NODES=${1:-20}
SPEED=${2:-10}
SCRIPT="scratch/aodv-test"

echo ""
echo "═══════════════════════════════════════════════"
echo "  Running: Nodes=$NODES, Speed=$SPEED m/s"
echo "  Blackhole: 0, 1, 2, 3"
echo "═══════════════════════════════════════════════"
echo ""

# Header
echo "BH | PDR(%)    | Throughput(Kbps) | Delay(ms)"
echo "---|-----------|------------------|----------"

for BH in 0 1 2 3; do
    # Run and capture CSV line
    RESULT=$(./ns3 run "$SCRIPT --nNodes=$NODES --nBlackhole=$BH --speed=$SPEED" 2>/dev/null | grep "^CSV:")
    
    # Parse CSV:  nodes,nBlackhole,speed,pdr,throughput,avgDelay
    PDR=$(echo "$RESULT" | cut -d',' -f4)
    TP=$(echo "$RESULT" | cut -d',' -f5)
    DELAY=$(echo "$RESULT" | cut -d',' -f6)
    
    printf " %d | %9s | %16s | %s\n" "$BH" "$PDR" "$TP" "$DELAY"
done

echo ""
echo "═══════════════════════════════════════════════"
echo "  Done! Nodes=$NODES, Speed=$SPEED"
echo "═══════════════════════════════════════════════"#!/bin/bash
# ═══════════════════════════════════════════════════
#  Run BH=0,1,2,3 for a given node count + speed
#
#  Usage: bash run-blackhole.sh [nNodes] [speed]
#  Example: bash run-blackhole.sh 20 10
#           bash run-blackhole.sh 40 0
# ═══════════════════════════════════════════════════

NODES=${1:-20}
SPEED=${2:-10}
SCRIPT="scratch/aodv-blackhole-test"

echo ""
echo "═══════════════════════════════════════════════"
echo "  Running: Nodes=$NODES, Speed=$SPEED m/s"
echo "  Blackhole: 0, 1, 2, 3"
echo "═══════════════════════════════════════════════"
echo ""

# Header
echo "BH | PDR(%)    | Throughput(Kbps) | Delay(ms)"
echo "---|-----------|------------------|----------"

for BH in 0 1 2 3; do
    # Run and capture CSV line
    RESULT=$(./ns3 run "$SCRIPT --nNodes=$NODES --nBlackhole=$BH --speed=$SPEED" 2>/dev/null | grep "^CSV:")
    
    # Parse CSV:  nodes,nBlackhole,speed,pdr,throughput,avgDelay
    PDR=$(echo "$RESULT" | cut -d',' -f4)
    TP=$(echo "$RESULT" | cut -d',' -f5)
    DELAY=$(echo "$RESULT" | cut -d',' -f6)
    
    printf " %d | %9s | %16s | %s\n" "$BH" "$PDR" "$TP" "$DELAY"
done

echo ""
echo "═══════════════════════════════════════════════"
echo "  Done! Nodes=$NODES, Speed=$SPEED"
echo "═══════════════════════════════════════════════"