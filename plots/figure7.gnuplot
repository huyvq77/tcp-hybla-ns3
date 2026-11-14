set terminal pngcairo size 1200,750 font "Sans,12"
set output "plots/figure7-goodput.png"
set title "Figure 7 - Single-flow TCP goodput with 1% wireless PER per direction"
set xlabel "Target propagation RTT (ms)"
set ylabel "Foreground goodput (Mbps)"
set xrange [0:625]
set logscale y
set yrange [0.04:10]
set xtics (25, 50, 100, 200, 300, 450, 600)
set ytics (0.05, 0.1, 0.5, 1, 5, 10)
set grid xtics ytics
set key top right

plot "data/figure7-goodput.dat" using 1:2 with linespoints lw 2 pt 7 \
         title "Hybla (patched) + SACK", \
     "" using 1:3 with linespoints lw 2 pt 5 \
         title "NewReno + SACK"
