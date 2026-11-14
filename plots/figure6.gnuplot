set terminal pngcairo size 1200,750 font "Sans,12"
set output "plots/figure6-goodput.png"
set title "Figure 6 - Foreground TCP goodput under congestion"
set xlabel "Target propagation RTT (ms)"
set ylabel "Foreground goodput (Mbps)"
set xrange [0:625]
set logscale y
set yrange [0.08:10]
set xtics (25, 50, 100, 200, 300, 450, 600)
set ytics (0.1, 1, 10)
set grid xtics ytics
set key top right

plot "data/figure6-goodput.dat" using 1:2 with linespoints lw 2 pt 7 \
         title "Hybla (patched) + SACK", \
     "" using 1:3 with linespoints lw 2 pt 5 \
         title "NewReno + SACK", \
     10.0/6.0 with lines dt 2 lw 2 \
         title "Nominal fair share (10/6 Mbps)"
