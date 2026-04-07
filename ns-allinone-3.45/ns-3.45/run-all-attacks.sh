

SCRIPT="scratch/final_simulation_script"
SIM_TIME=100
SEED=30

# 5 scenarios
#SCENARIOS=("grayhole_none" "grayhole_trust" "smartgray_none" "smartgray_trust")

# Default values (when not varying)
DEF_NODES=40
DEF_FLOWS=20
DEF_PPS=200
DEF_AREA=3

echo "============================================"
echo " 802.11 Static - Full Experiment Suite"
echo " 4 experiments x 5 datapoints x 5 scenarios"
echo " Total: 100 simulation runs"
echo "============================================"

# ============================================================
# EXPERIMENT 1: Vary Nodes (20, 40, 60, 80, 100)
# ============================================================
run_exp1_nodes() {
    SCENARIOS=("grayhole_trust" "smartgray_none" "smartgray_trust")
    OUTFILE="results_wifi_nodes.csv"
    echo "nNodes,nFlows,pps,areaMult,scenario,throughput,delay,pdr,dropRatio,energy" > $OUTFILE
    echo ""
    echo "===== EXPERIMENT 1: Vary Nodes ====="
    
    NODES_LIST=(20 40 60 80 100)
    total=${#NODES_LIST[@]}
    
    for scenario in "${SCENARIOS[@]}"; do
        count=0
        for n in "${NODES_LIST[@]}"; do
            count=$((count + 1))
            echo "[Exp1] Scenario=$scenario | Nodes=$n ($count/$total)"
            ./ns3 run "$SCRIPT --nNodes=$n --nFlows=$DEF_FLOWS --pps=$DEF_PPS --areaMult=$DEF_AREA --scenario=$scenario --simTime=$SIM_TIME --seed=$SEED --outputFile=$OUTFILE" 2>/dev/null
        done
    done
    echo "===== Exp1 DONE! Results in $OUTFILE ====="
}

# ============================================================
# EXPERIMENT 2: Vary Flows (10, 20, 30, 40, 50)
# ============================================================
run_exp2_flows() {
    SCENARIOS=("grayhole_trust" "smartgray_none" "smartgray_trust")
    OUTFILE="results_wifi_flows.csv"
    echo "nNodes,nFlows,pps,areaMult,scenario,throughput,delay,pdr,dropRatio,energy" > $OUTFILE
    echo ""
    echo "===== EXPERIMENT 2: Vary Flows ====="
    
    FLOWS_LIST=(10 20 30 40 50)
    total=${#FLOWS_LIST[@]}
    
    for scenario in "${SCENARIOS[@]}"; do
        count=0
        for f in "${FLOWS_LIST[@]}"; do
            count=$((count + 1))
            echo "[Exp2] Scenario=$scenario | Flows=$f ($count/$total)"
            ./ns3 run "$SCRIPT --nNodes=$DEF_NODES --nFlows=$f --pps=$DEF_PPS --areaMult=$DEF_AREA --scenario=$scenario --simTime=$SIM_TIME --seed=$SEED --outputFile=$OUTFILE" 2>/dev/null
        done
    done
    echo "===== Exp2 DONE! Results in $OUTFILE ====="
}

# ============================================================
# EXPERIMENT 3: Vary Packets/sec (100, 200, 300, 400, 500)
# ============================================================
run_exp3_pps() {
    SCENARIOS=("grayhole_trust" "smartgray_none" "smartgray_trust")
    OUTFILE="results_wifi_pps.csv"
    echo "nNodes,nFlows,pps,areaMult,scenario,throughput,delay,pdr,dropRatio,energy" > $OUTFILE
    echo ""
    echo "===== EXPERIMENT 3: Vary Packets/sec ====="
    
    PPS_LIST=(100 200 300 400 500)
    total=${#PPS_LIST[@]}
    
    for scenario in "${SCENARIOS[@]}"; do
        count=0
        for p in "${PPS_LIST[@]}"; do
            count=$((count + 1))
            echo "[Exp3] Scenario=$scenario | PPS=$p ($count/$total)"
            ./ns3 run "$SCRIPT --nNodes=$DEF_NODES --nFlows=$DEF_FLOWS --pps=$p --areaMult=$DEF_AREA --scenario=$scenario --simTime=$SIM_TIME --seed=$SEED --outputFile=$OUTFILE" 2>/dev/null
        done
    done
    echo "===== Exp3 DONE! Results in $OUTFILE ====="
}

# ============================================================
# EXPERIMENT 4: Vary Coverage Area (1x, 2x, 3x, 4x, 5x Tx_range)
# ============================================================
run_exp4_area() {
    SCENARIOS=("grayhole_trust" "smartgray_none" "smartgray_trust")
    OUTFILE="results_wifi_area.csv"
    echo "nNodes,nFlows,pps,areaMult,scenario,throughput,delay,pdr,dropRatio,energy" > $OUTFILE
    echo ""
    echo "===== EXPERIMENT 4: Vary Coverage Area ====="
    
    AREA_LIST=(1 2 3 4 5)
    total=${#AREA_LIST[@]}
    
    for scenario in "${SCENARIOS[@]}"; do
        count=0
        for a in "${AREA_LIST[@]}"; do
            count=$((count + 1))
            echo "[Exp4] Scenario=$scenario | AreaMult=$a ($count/$total)"
            ./ns3 run "$SCRIPT --nNodes=$DEF_NODES --nFlows=$DEF_FLOWS --pps=$DEF_PPS --areaMult=$a --scenario=$scenario --simTime=$SIM_TIME --seed=$SEED --outputFile=$OUTFILE" 2>/dev/null
        done
    done
    echo "===== Exp4 DONE! Results in $OUTFILE ====="
}

# ============================================================
# Run all or specific experiment
# ============================================================
if [ "$1" == "1" ]; then
    run_exp1_nodes
elif [ "$1" == "2" ]; then
    run_exp2_flows
elif [ "$1" == "3" ]; then
    run_exp3_pps
elif [ "$1" == "4" ]; then
    run_exp4_area
else
    echo ""
    echo "Usage:"
    echo "  Run ALL:          bash run_all_wifi.sh all"
    echo "  Run Experiment 1: bash run_all_wifi.sh 1   (Vary Nodes)"
    echo "  Run Experiment 2: bash run_all_wifi.sh 2   (Vary Flows)"
    echo "  Run Experiment 3: bash run_all_wifi.sh 3   (Vary PPS)"
    echo "  Run Experiment 4: bash run_all_wifi.sh 4   (Vary Area)"
    echo ""
    echo "  Parallel (4 terminals):"
    echo "    Terminal 1: bash run_all_wifi.sh 1"
    echo "    Terminal 2: bash run_all_wifi.sh 2"
    echo "    Terminal 3: bash run_all_wifi.sh 3"
    echo "    Terminal 4: bash run_all_wifi.sh 4"
    echo ""
    
    if [ "$1" == "all" ]; then
        run_exp1_nodes
        run_exp2_flows
        run_exp3_pps
        run_exp4_area
        echo ""
        echo "============================================"
        echo " ALL EXPERIMENTS COMPLETE!"
        echo " Results:"
        echo "   results_wifi_nodes.csv"
        echo "   results_wifi_flows.csv"
        echo "   results_wifi_pps.csv"
        echo "   results_wifi_area.csv"
        echo "============================================"
    fi
fi