set terminal pngcairo size 1200,750 font "Sans,12"
set output "plots/figure8-goodput.png"
set title "Figure 8 - TCP Hybla under congestion and wireless packet loss"
set xlabel "Target propagation RTT (ms)"
set ylabel "Foreground goodput (Mbps)"
set xrange [0:625]
set logscale y
set yrange [0.4:10]
set xtics (25, 50, 100, 200, 300, 450, 600)
set ytics (0.5, 1, 2, 5, 10)
set grid xtics ytics
set key top right

plot "data/figure8-goodput.dat" using 1:2 with linespoints lw 2 pt 7 title "Hybla, PER 0%", \
     "" using 1:3 with linespoints lw 2 pt 5 title "Hybla, PER 1%", \
     "" using 1:4 with linespoints lw 2 pt 9 title "Hybla, PER 5%", \
     10.0/6.0 with lines dt 2 lw 2 lc rgb "#666666" title "Nominal fair share (10/6 Mbps)"
