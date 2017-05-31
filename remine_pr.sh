RUNS=$1
OUTPUTS=$2
for ((i = 1; i <= $RUNS; i++));
do
	bash remine_seg.sh $i $OUTPUTS
done