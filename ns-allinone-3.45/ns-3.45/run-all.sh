#!/bin/bash
# Run all attack + defense combinations
# Output: results.csv

OUTPUT="results_v3.csv"
echo "nNodes,attack,nMalicious,defense,speed,pdr,throughput,routingOverhead,nrl" > $OUTPUT

NODES=50
SPEEDS="5 15 25 35"

echo "============================================"
echo " Running all simulations (nNodes=$NODES)"
echo "============================================"

for atk in blackhole grayhole; do
    for nmal in 1 2; do
        echo "--- $atk (nMalicious=$nmal, Trust-based-AODV) ---"
        for s in $SPEEDS; do
            echo "  speed=$s"
            CMD="scratch/aodv_bh_gh --nNodes=$NODES --attack=$atk --nMalicious=$nmal --defense=trust --speed=$s"
            if [[ "$atk" == "grayhole" || "$atk" == "smartgray" ]]; then
                CMD="$CMD --dropProb=0.5"
            fi
            ./ns3 run "$CMD" 2>&1 | grep "^CSV:" | sed 's/CSV://' >> $OUTPUT
        done
    done
done

echo ""
echo "============================================"
echo " Done! Results saved to: $OUTPUT"
echo "============================================"
echo ""
echo "Total rows:"
wc -l $OUTPUT