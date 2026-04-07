#!/bin/bash
#
# Script to run Black Hole attack experiments with varying node counts
# This script automates running simulations for different configurations
#

# Configuration
NODE_COUNTS=(10 20 30 40 50)
DURATION=40
OUTPUT_DIR="blackhole_results"

# Create output directory
mkdir -p $OUTPUT_DIR

# Clear previous results file
RESULTS_FILE="$OUTPUT_DIR/summary_results.csv"
echo "Nodes,BlackHoleEnabled,PDR(%),Throughput(kbps),AvgDelay(ms),TxPackets,RxPackets" > $RESULTS_FILE

echo "========================================="
echo "Black Hole Attack Experiment Suite"
echo "========================================="
echo ""

# Run experiments for each node count
for nodes in "${NODE_COUNTS[@]}"; do
    echo "Running experiments with $nodes nodes..."
    
    # Calculate malicious node ID (middle node)
    malicious_node=$((nodes / 2))
    
    # Run WITHOUT Black Hole attack (baseline)
    echo "  [1/2] Running baseline (no attack)..."
    ./ns3 run "aodv-blackhole-test --nodes=$nodes --duration=$DURATION --enableBlackHole=false --output=$OUTPUT_DIR/baseline_${nodes}nodes" > /dev/null 2>&1
    
    if [ $? -eq 0 ]; then
        echo "  ✓ Baseline completed"
    else
        echo "  ✗ Baseline failed"
    fi
    
    # Run WITH Black Hole attack
    echo "  [2/2] Running with Black Hole attack..."
    ./ns3 run "aodv-blackhole-test --nodes=$nodes --duration=$DURATION --enableBlackHole=true --maliciousNode=$malicious_node --output=$OUTPUT_DIR/blackhole_${nodes}nodes" > /dev/null 2>&1
    
    if [ $? -eq 0 ]; then
        echo "  ✓ Black Hole attack completed"
    else
        echo "  ✗ Black Hole attack failed"
    fi
    
    echo ""
done

echo "========================================="
echo "All experiments completed!"
echo "Results saved in: $OUTPUT_DIR/"
echo "Summary file: $RESULTS_FILE"
echo "========================================="
echo ""

# Display summary
echo "Summary of Results:"
echo "-------------------"
cat $RESULTS_FILE | column -t -s,

echo ""
echo "To analyze and plot results, run:"
echo "  python3 analyze-blackhole-results.py"
