#!/bin/bash

makePlot()
{
	gnuplot <<- EOF
		set xlabel "Nodes"
		set ylabel "$1"
		set terminal svg
		set output "$2.svg"
                plot  '$3.dat' using 1:2:3 w yerrorbars title "Standardna odchylka", '' w linespoints title "Priemer"
	EOF
}

makePlotPacket()
{
	gnuplot <<- EOF
		set xlabel "Size of packet K"
		set ylabel "$1"
		set terminal svg
		set output "$2.svg"
		plot  '$3.dat' using 1:2:3 w yerrorbars title "Standardna odchylka", '' w linespoints title "Priemer"
	EOF
}


makeAverage()
{
	declare -a argArr=("${!1}")
	AVERAGE=0
	for i in "${argArr[@]}"
	do
		AVERAGE=$(bc -l <<< "$AVERAGE+$i")
	done
	LENGTH=${#argArr[@]}
	echo $(bc -l <<< "$AVERAGE/$LENGTH")
}


makeStDev()
{
declare -a argArr=("${!1}")
        daco=100
	suma=0
        rowa=0
        sumasq=1
        sqrsum=0
        minu1=0
        minu2=0
        minu2a=0
        celkmin=0
        odchylka=0
        medzi=0
        avgg=0
	for i in "${argArr[@]}"

	do
            suma=$(bc -l <<< "$suma+$i")
            medzi=$(bc -l <<< "$i*$i")
            sumasq=$(bc -l <<< "$sumasq+($medzi)")
        done
         LENGTH=${#argArr[@]}
         minu1=$(bc -l <<< "$sumasq/$LENGTH")
         minu2=$(bc -l <<< "$suma/$LENGTH")
         minu2a=$(bc -l <<< "$minu2*$minu2")
         celkmin=$(bc -l <<< "$minu1-$minu2a")
         odchylka=$(echo "sqrt ($celkmin)" | bc -l) 
echo "$odchylka" 
}


THROUGHPUT_FILE=throughput
PACKETSIZE_FILE=packetSize
UZITOCNE_FILE=uzitocneData


echo Start

for i in $(seq 11 11 55) #55 
do
	for j in $(seq 1 5) #5
	do
		OUTPUT=$(./waf --run "scratch/zadanie --helloInterval=$i --lostNodes")
		#NODES=$(echo $OUTPUT | grep -o "Nodes: [0-9]\+" | grep -o "[0-9]\+")
		
		THROUGHPUT=$(echo $OUTPUT | grep -o "Lost-nodes: [0-9]\+[.]\?[0-9]*" | grep -o "[0-9]\+[.]\?[0-9]*")
		echo "$i $THROUGHPUT" >> "$THROUGHPUT_FILE$j.dat"
		THROUGHPUT_ARRAY[$j]=$THROUGHPUT
		#UZITOCNE=$(echo $OUTPUT | grep -o "UzitocneData: [0-9]\+[.]\?[0-9]*" | grep -o "[0-9]\+[.]\?[0-9]*")
		#echo "$NODES $UZITOCNE" >> "$UZITOCNE_FILE$j.dat"
		#UZITOCNE_ARRAY[$j]=$UZITOCNE 
	done
	DATA=$(makeAverage THROUGHPUT_ARRAY[@])
	SMODCH=$(makeStDev THROUGHPUT_ARRAY[@])
	echo "$NODES $DATA $SMODCH" >> $THROUGHPUT_FILE"_avg.dat"
	#DATA_N=$(makeAverage UZITOCNE_ARRAY[@])
	#SMODCH=$(makeStDev UZITOCNE_ARRAY[@])
	#echo "$NODES $DATA_N $SMODCH" >> $UZITOCNE_FILE"_avg.dat"
done


for o in $(seq 10 6 34) 
do
	for p in $(seq 1 5) #5
	do
		OUTPUT_PACKET=$(./waf --run "scratch/zadanie --packetSize=$o")
		PACKETSIZE=$(echo $OUTPUT_PACKET | grep -o "packetSize: [0-9]\+" | grep -o "[0-9]\+")
		THROUGHPUT=$(echo $OUTPUT_PACKET | grep -o "Throughput: [0-9]\+[.]\?[0-9]*" | grep -o "[0-9]\+[.]\?[0-9]*")
		echo "$PACKETSIZE $THROUGHPUT" >> "$PACKETSIZE_FILE$p.dat"
		PACKETSIZE_ARRAY[$p]=$THROUGHPUT
		
	done
	DATA=$(makeAverage PACKETSIZE_ARRAY[@])
	SMODCH=$(makeStDev PACKETSIZE_ARRAY[@])
	echo "$PACKETSIZE $DATA $SMODCH" >> $PACKETSIZE_FILE"_avg.dat"
	
done



makePlot "Throughput" "ThroughputNodes" $THROUGHPUT_FILE"_avg" 
#makePlot "UzitocneData" "UzitocneNodes" $UZITOCNE_FILE"_avg" 
#makePlotPacket "Throughput" "ThroughputDatas" $PACKETSIZE_FILE"_avg"

echo Stop


