#!/bin/bash
# ═══════════════════════════════════════════════
#  Run ALL scenarios: nodes × blackhole
#  Output: results.csv
# ═══════════════════════════════════════════════

SCRIPT="scratch/aodv-test"
SIMTIME=600
SPEED=0

CSV="results2.csv"
echo "nodes,blackhole,speed,pdr,throughput,delay" > "$CSV"

for NODES in 20 40 60 80 100; do
    for BH in 0 1 2 3; do
        echo ">>> Running: Nodes=$NODES  BH=$BH  Speed=$SPEED ..."
        
        OUTPUT=$(./ns3 run "$SCRIPT --nNodes=$NODES --nBlackhole=$BH --speed=$SPEED --simTime=$SIMTIME" 2>/dev/null | grep "^CSV:")
        
        # Parse: CSV:nodes,blackhole,speed,pdr,throughput,delay
        PDR=$(echo "$OUTPUT" | cut -d',' -f4)
        TP=$(echo "$OUTPUT" | cut -d',' -f5)
        DELAY=$(echo "$OUTPUT" | cut -d',' -f6)
        
        echo "$NODES,$BH,$SPEED,$PDR,$TP,$DELAY" >> "$CSV"
        echo "    PDR=$PDR%  TP=$TP  Delay=$DELAY"
    done
done

echo ""
echo "═══════════════════════════════════════════════"
echo " DONE! Results saved to: $CSV"
echo "═══════════════════════════════════════════════"
echo ""
cat "$CSV"


# 2. Attacks with 1 and 2 malicious nodes (no defense)
for atk in blackhole grayhole smartblack smartgray; do
    for nmal in 1 2; do
        echo "--- $atk (nMalicious=$nmal, no defense) ---"
        for s in $SPEEDS; do
            echo "  speed=$s"
            CMD="scratch/aodv_bh_gh --nNodes=$NODES --attack=$atk --nMalicious=$nmal --speed=$s"
            if [[ "$atk" == "grayhole" || "$atk" == "smartgray" ]]; then
                CMD="$CMD --dropProb=0.5"
            fi
            ./ns3 run "$CMD" 2>&1 | grep "^CSV:" | sed 's/CSV://' >> $OUTPUT
        done
    done
done

# 3. Attacks with IDS-AODV defense
#for atk in blackhole grayhole smartblack smartgray; do
 #   for nmal in 1 2; do
  #      echo "--- $atk (nMalicious=$nmal, IDS-AODV) ---"
        for s in $SPEEDS; do
            echo "  speed=$s"
            CMD="scratch/aodv_bh_gh --nNodes=$NODES --attack=$atk --nMalicious=$nmal --defense=ids --speed=$s"
            if [[ "$atk" == "grayhole" || "$atk" == "smartgray" ]]; then
                CMD="$CMD --dropProb=0.5"
            fi
            ./ns3 run "$CMD" 2>&1 | grep "^CSV:" | sed 's/CSV://' >> $OUTPUT
        done
    done
done

# 4. Attacks with MBDP-AODV defense
for atk in blackhole grayhole smartblack smartgray; do
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