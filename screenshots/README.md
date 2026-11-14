# Tra cứu ảnh minh chứng

Các lệnh `./ns3 run` và `NS_LOG` trong bảng chạy từ thư mục ns-3.49. Hai script Bash và các lệnh Gnuplot chạy từ thư mục gốc của gói. Xem [README](../README.md) để cài mã trước khi chạy lại.

`run-figures.sh` và `run-checks.sh` là các script chạy lại được gom sau khi thực nghiệm. Bảng ghi cấu hình thấy trong ảnh, lệnh tương đương hoặc script chạy lại. Với các ảnh lịch sử, cột cuối ghi rõ khi chỉ còn ảnh hoặc khi số liệu trong gói đến từ lượt đối chứng khác.

| Ảnh | Lệnh hoặc cách tạo | Kết quả và nguồn lưu |
|---|---|---|
| [01](01-hybla-rrtt-rho.png) | Mở `src/internet/model/tcp-hybla.cc` trong Mousepad, xem `GetTypeId()` và `RecalcParam()` | Mã Hybla ở giai đoạn khảo sát ban đầu. |
| [02](02-hybla-cwnd-formulas.png) | Mở cùng tệp, xem `SlowStart()` và `CongestionAvoidance()` | Ảnh mã gốc, trước khi chỉnh Slow Start. Xem [diff](../patches/tcp-hybla.patch). |
| [03](03-hybla-simulation-tracing.png) | `./ns3 run "tcp-variants-comparison --transport_prot=TcpHybla --duration=30 --tracing=1"` | Tạo `TcpVariantsComparison-cwnd.data` và các tệp trace khác. Trace của lượt thăm dò này không có trong gói. |
| [04](04-hybla-cwnd.png) | Dùng Gnuplot vẽ thời gian và cửa sổ từ `TcpVariantsComparison-cwnd.data` của ảnh 03 | Đồ thị cửa sổ của ví dụ TCP, lưu dưới dạng ảnh. |
| [05](05-hybla-rrtt-25ms.png) | `./ns3 run scratch/hybla-config-test` | TCP Hybla, RTT tham chiếu 25 ms. `run-checks.sh` tạo `basic/config.log`. |
| [06](06-hybla-rho-verification.png) | `./ns3 run scratch/hybla-rho-test` | Kiểm tra hệ số ρ. `run-checks.sh` tạo `basic/rho.log`. |
| [07](07-figure5-connectivity.png) | `./ns3 run "scratch/hybla-figure5 --rtt=100"` | Kiểm tra kết nối UDP Echo. `run-checks.sh` tạo `basic/figure5.log`. |
| [08](08-figure6-hybla-pilot.png) | `./ns3 run "scratch/hybla-experiment --rtt=300 --tcp=TcpHybla --duration=30"` | Lượt thăm dò Figure 6 trước khi chỉnh mã. Còn ảnh. |
| [09](09-figure6-newreno-pilot.png) | `./ns3 run "scratch/hybla-experiment --rtt=300 --tcp=TcpNewReno --duration=30"` | Lượt NewReno thăm dò, 30 giây. Còn ảnh. |
| [10](10-figure6-newreno-rtt25.png) | Chạy Hybla rồi NewReno với `--rtt=25 --duration=30` | Kết quả đối chiếu ở giai đoạn thăm dò. Còn ảnh. |
| [11](11-figure6-hybla-rtt25.png) | `./ns3 run "scratch/hybla-experiment --rtt=25 --tcp=TcpHybla --duration=30"` | Hybla tại RTT 25 ms, 30 giây. Còn ảnh. |
| [12](12-figure6-hybla-rtt300-600s.png) | `./ns3 run "scratch/hybla-experiment --rtt=300 --tcp=TcpHybla --duration=600"` trước khi chỉnh mã | Lượt cho khoảng 0,29267 Mbps. Kết quả sau khi sửa ở ảnh 18. |
| [13](13-hybla-rho-condition-before-fix.png) | Chạy Hybla với `--rtt=300 --duration=30`, rồi lọc log bằng `grep -F HYBLA_RTT_CHECK` | Ảnh từ mã lịch sử có dòng chẩn đoán tạm. Xem ghi chú dưới bảng. |
| [14](14-hybla-rho12-after-fix.png) | Chạy Hybla sau khi cập nhật cách tính ρ với `--rtt=300 --duration=30`, bật log `TcpHybla` | Trace ρ = 12. Lượt tương đương trong [cwnd-startup](../data/cwnd-startup/). |
| [15](15-hybla-rho24-slowstart-check.png) | Chạy Hybla với `--rtt=600 --duration=30`, bật log `TcpHybla` | Trace ρ = 24 và bước tăng cửa sổ đầu tiên. Lượt tương đương trong [cwnd-startup](../data/cwnd-startup/). |
| [16](16-figure6-hybla-rtt600-600s.png) | Figure 6, Hybla, RTT 600 ms, 600 giây | [Log đối chứng](../data/figure6/TcpHybla-rtt600-600s.log.gz) và [bảng goodput](../data/figure6-goodput.dat). |
| [17](17-figure6-newreno-rtt600-600s.png) | Figure 6, NewReno, RTT 600 ms, 600 giây | Ảnh và [bảng goodput](../data/figure6-goodput.dat). |
| [18](18-figure6-rtt300-comparison.png) | Figure 6, Hybla và NewReno, RTT 300 ms, 600 giây | Ảnh và [bảng goodput](../data/figure6-goodput.dat). |
| [19](19-figure6-rtt25-comparison.png) | Figure 6, Hybla và NewReno, RTT 25 ms, 600 giây | Hai log đối chứng tại [data/figure6](../data/figure6/) và [bảng goodput](../data/figure6-goodput.dat). |
| [20](20-figure6-rtt-sweep.png) | Vòng lặp Bash gốc chạy hai TCP tại 50, 100, 200, 450 ms. Chạy lại cả hình bằng `FIGURES=6 bash run-figures.sh "$ns3_dir"` | Tám log tương ứng trong [data/figure6](../data/figure6/). |
| [21](21-figure6-goodput-plot.png) | `gnuplot plots/figure6.gnuplot` | [Đồ thị](../plots/figure6-goodput.png), từ [bảng goodput](../data/figure6-goodput.dat). |
| [22](22-figure7-single-flow-baseline.png) | `./ns3 run "scratch/hybla-experiment --rtt=300 --tcp=TcpHybla --duration=30 --background=0"` | Lượt luồng đơn với PER 0%, goodput 8,60567 Mbps. |
| [23](23-figure7-hybla-per1-pilot.png) | Lệnh [chạy nhanh](../README.md#chạy-nhanh) với PER 1% mỗi chiều | Lượt thử luồng đơn trước khi quét RTT, lưu bằng ảnh. |
| [24](24-figure7-per1-bidirectional-drops.png) | Lệnh [chạy nhanh](../README.md#chạy-nhanh) | Bộ đếm lỗi hai chiều 242 và 217, lưu bằng ảnh. |
| [25](25-figure7-rtt-sweep.png) | Vòng lặp Bash gốc. Chạy lại bằng `FIGURES=7 bash run-figures.sh "$ns3_dir"` | [Bảng tổng hợp và 14 log](../data/figure7/). |
| [26](26-figure7-goodput-plot.png) | `gnuplot plots/figure7.gnuplot` | [Đồ thị](../plots/figure7-goodput.png), từ [bảng goodput](../data/figure7-goodput.dat). |
| [27](27-figure8-rtt300-per1-pilot.png) | Chạy Hybla với `--rtt=300 --duration=600 --background=1 --per=0.01 --sack=1` | [Log](../data/figure8/TcpHybla-rtt300-per1-600s.log) dùng lại trong đợt quét. |
| [28](28-figure8-per-sweep.png) | Vòng lặp Bash gốc. Chạy lại bằng `FIGURES=8 bash run-figures.sh "$ns3_dir"` | [Bảng tổng hợp và 14 log](../data/figure8/) của PER 1% và 5%. |
| [29](29-figure8-goodput-plot.png) | `gnuplot plots/figure8.gnuplot` | [Đồ thị](../plots/figure8-goodput.png), từ [bảng goodput](../data/figure8-goodput.dat). |
| [30](30-source-package-smoke-test.png) | Lệnh [chạy nhanh](../README.md#chạy-nhanh) sau khi chép mã từ gói vào ns-3 | Goodput 3,50609 Mbps, lỗi 242 và 217, RED hai chiều bằng 0. |
| [31](31-ack25-goodput-comparison.png) | `run-checks.sh`, nhóm `ack25`, Hybla và NewReno với ACK mặc định và `DelAckCount=1` | [Bốn log đối chiếu](../data/ack25/). |
| [32](32-hybla-rho-rtt25.png) | `run-checks.sh`, lượt `ack25/hybla-rho25` | [Log ρ tại 25 ms](../data/ack25/hybla-rho25.log). |
| [33](33-hybla-cwnd-ssthresh-startup.png) | `run-checks.sh`, nhóm `cwnd-startup` tại 25, 300, 600 ms | [Ba log khởi động](../data/cwnd-startup/). |
| [34](34-hybla-ssthresh600-comparison.png) | `run-checks.sh`, nhóm `ssthresh600`, ngưỡng mặc định và 750.000 byte | [Hai log đối chiếu](../data/ssthresh600/). |
| [35](35-newreno-mss1024-figure7.png) | `run-checks.sh`, nhóm `mss1024`, NewReno với MSS 1.024 byte tại 25 và 300 ms | [Hai log](../data/mss1024/), goodput 1,898 và 0,187392 Mbps. |

Ảnh 13 ghi giá trị RTT tức thời và RTT nhỏ nhất từ dòng `HYBLA_RTT_CHECK` được thêm tạm để kiểm tra điều kiện cập nhật ρ. Đảo [bản vá](../patches/tcp-hybla.patch) trên một bản sao ns-3.49 khôi phục điều kiện gốc. Dòng chẩn đoán tạm cần được thêm riêng nếu muốn in đúng các trường như trong ảnh.
