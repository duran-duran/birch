set term png large enhanced size 1024, 768
set output 'performance.png'
set palette defined ( 0 "blue", 1 "green", 2 "yellow", 3 "red", 4 "red", 5 "red" );
set xlabel "Number of processes";
set ylabel "Number of data points";
set zlabel "Time (ms)";
set title "BIRCH algorithm performance";
set key off;
set pm3d;
splot "parallel_results.txt" with lines;
