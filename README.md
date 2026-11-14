# Mô phỏng TCP Hybla trên ns-3.49

Bài tập lớn môn Đánh giá hiệu năng mạng. Bài tập mô phỏng các kịch bản của Figure 6–8 trên ns-3.49 để đối chiếu với kết quả trong bài báo *TCP Hybla: a TCP enhancement for heterogeneous networks* (Caini và Firrincieli, 2004). Mô hình dựa trên Figure 5 của bài báo: một luồng khảo sát có RTT thay đổi từ 25 đến 600 ms, cùng 0 hoặc 5 luồng NewReno nền chia sẻ nút nghẽn 10 Mbps. Đại lượng đo là goodput, tức tốc độ dữ liệu ứng dụng nhận được ở đích của luồng khảo sát. [Bài báo gốc](https://doi.org/10.1002/sat.799)

PER là xác suất lỗi gói trên mỗi chiều của liên kết G–D0, áp dụng cho chiều dữ liệu và chiều ACK.

| Figure | Luồng khảo sát | Luồng nền | PER mỗi chiều | Đồ thị trong báo cáo | Bảng dữ liệu |
|---|---|---:|---:|---|---|
| 6 | Hybla và NewReno | 5 | 0% | Hình III.2 | [`figure6-goodput.dat`](data/figure6-goodput.dat) |
| 7 | Hybla và NewReno | 0 | 1% | Hình III.4 | [`figure7-goodput.dat`](data/figure7-goodput.dat) |
| 8 | Hybla | 5 | 0%, 1%, 5% | Hình III.6 | [`figure8-goodput.dat`](data/figure8-goodput.dat) |

Mỗi cấu hình chạy một lượt mô phỏng 600 giây với SACK bật. Bảy mức RTT lần lượt là 25, 50, 100, 200, 300, 450 và 600 ms. Mỗi hình cần 14 lượt mới, tổng cộng 42 lượt. Đường Hybla với PER 0% của Figure 8 dùng lại kết quả từ Figure 6.

## Nội dung gói

```text
tcp-hybla-ns3/
├── README.md
├── .gitignore
├── scratch/                     mô phỏng chính và ba chương trình kiểm tra
├── src/internet/model/          tcp-hybla.cc đã chỉnh, tcp-hybla.h giữ nguyên
├── patches/tcp-hybla.patch      khác biệt so với ns-3.49 gốc
├── run-figures.sh               chạy 42 lượt Figure 6–8, tổng hợp bảng, vẽ đồ thị
├── run-checks.sh                chạy các kiểm tra cấu hình và phép thử bổ sung
├── compare-results.sh           đối chiếu bảng goodput
├── data/                        log và bảng goodput đã lưu
├── plots/                       script Gnuplot, Matplotlib và đồ thị
├── screenshots/                 35 ảnh minh chứng và bảng tra lệnh
└── COPYING-GPL-2.0
```

| Chương trình trong `scratch/` | Việc làm |
|---|---|
| `hybla-experiment.cc` | Mô hình Figure 5, đo goodput và đếm số gói bị loại |
| `hybla-config-test.cc` | Kiểm tra loại TCP và RTT tham chiếu 25 ms |
| `hybla-rho-test.cc` | Kiểm tra hệ số ρ với RTT nhỏ nhất 25, 50, 100, 200 ms (kỳ vọng 1, 2, 4, 8) |
| `hybla-figure5.cc` | Kiểm tra địa chỉ và định tuyến bằng UDP Echo |

Tham số của `hybla-experiment`

| Tham số | Ý nghĩa |
|---|---|
| `--rtt` | RTT lan truyền mục tiêu của luồng khảo sát, tính bằng ms, tối thiểu 20 |
| `--tcp` | `TcpHybla` hoặc `TcpNewReno` cho luồng khảo sát |
| `--duration` | Thời gian đo goodput, tính bằng giây mô phỏng, lớn hơn 0 |
| `--per` | Xác suất lỗi gói mỗi chiều, từ 0 đến 1. Ví dụ `--per=0.01` tương ứng 1% |
| `--background` | `1` bật cả 5 luồng NewReno nền, `0` tắt cả 5 |
| `--sack` | `1` bật SACK, `0` tắt SACK |

## Cài đặt

Chạy các lệnh trong Bash trên Linux với ns-3.49 đã biên dịch được. Cài Gnuplot nếu muốn vẽ đồ thị. Mở Bash ở thư mục gốc của gói, đặt `ns3_dir` theo máy của bạn và dùng cùng phiên Bash cho các bước tiếp theo.

```bash
ns3_dir="$HOME/Projects/ns3/ns-3.49"
cp --backup=numbered scratch/hybla-*.cc "$ns3_dir/scratch/"
cp --backup=numbered src/internet/model/tcp-hybla.{cc,h} "$ns3_dir/src/internet/model/"
```

Lần chạy đầu tiên có thể cần biên dịch lại. Với ns-3.49 gốc, có thể thay bước chép hai tệp Hybla bằng `patch -p1 -d "$ns3_dir" < patches/tcp-hybla.patch`. Bước chép các tệp trong `scratch/` vẫn cần.

## Chạy nhanh

Một luồng Hybla, RTT 300 ms, PER 1% mỗi chiều, 30 giây mô phỏng.

```bash
(cd "$ns3_dir" && NS_LOG= ./ns3 run "scratch/hybla-experiment --rtt=300 --tcp=TcpHybla --duration=30 --background=0 --per=0.01 --sack=1 --RngSeed=1 --RngRun=1")
```

Kết quả tham chiếu trên Kali Linux được ghi ở [ảnh 30](screenshots/30-source-package-smoke-test.png).

```text
Foreground[0] rxBytes=13147848 goodputMbps=3.50609
RED[0] droppedPackets=0
RED[1] droppedPackets=0
Wireless drops forward (gateway -> sink): 242
Wireless drops reverse (sink -> gateway): 217
```

Các lệnh mô phỏng đặt `RngSeed=1` và `RngRun=1`, tương ứng mặc định ns-3.49. Dùng các bảng đã lưu trong `data/` để đối chiếu trên môi trường khác.

## Chạy lại toàn bộ

```bash
OUTPUT_DIR="$PWD/reproduced.main" bash run-figures.sh "$ns3_dir"
bash run-checks.sh "$ns3_dir"
```

| Script | Chạy | Kết quả |
|---|---|---|
| `run-figures.sh` | 42 lượt tuần tự, mỗi lượt 600 giây mô phỏng | Log và `summary.tsv` trong `<OUTPUT_DIR>/data/figureN/`, bảng `figureN-goodput.dat` trực tiếp trong `<OUTPUT_DIR>/data/`, đồ thị trong `<OUTPUT_DIR>/plots/` nếu có Gnuplot |
| `run-checks.sh` | 3 kiểm tra cơ bản, ACK trễ ở 25 ms, cửa sổ khởi động ở 25/300/600 ms, ngưỡng Slow Start ở 600 ms, MSS 1.024 byte | `reproduced.checks.*/{basic,ack25,cwnd-startup,ssthresh600,mss1024}/` |

`run-figures.sh` nhận hai biến môi trường:

- `FIGURES`: chọn hình cần chạy, ví dụ `FIGURES="7"` hoặc `FIGURES="6 8"`. Mặc định là `"6 7 8"`.
- `OUTPUT_DIR`: thư mục kết quả. Chạy lại với cùng thư mục thì script bỏ qua các lượt đã có log đầy đủ và chỉ chạy các lượt còn thiếu, nên có thể tiếp tục sau khi bị ngắt. Cần giữ nguyên mã và cấu hình giữa các lần chạy.

Khi chạy riêng Figure 8, đường PER 0% lấy từ [`data/figure6-goodput.dat`](data/figure6-goodput.dat). Khi chạy cùng Figure 6 trong một lần, script dùng đường PER 0% vừa tạo.

`run-checks.sh` ghi trace `cwnd` và ρ bằng `NS_LOG`, nên cần ns-3 build ở profile `default` hoặc `debug`. Script dừng với thông báo lỗi nếu log không có các dòng trace này.

## Đối chiếu với kết quả đã lưu

Sau khi chạy `run-figures.sh`, truyền số hình (6, 7 hoặc 8) và thư mục kết quả cho script.

```bash
bash compare-results.sh 7 reproduced.main
```

Script kiểm tra số cột, giá trị goodput và đủ bảy RTT theo đúng thứ tự. Script in chênh lệch tuyệt đối lớn nhất giữa các ô goodput tương ứng. Dưới đây là kết quả khi hai bảng Figure 7 trùng nhau.

```text
Figure 7: 7 rows compared, max |delta| = 0 Mbps
```

Script trả mã lỗi khi thiếu tệp hoặc dữ liệu không hợp lệ. Với hai bảng hợp lệ, bạn đọc giá trị chênh lệch để đối chiếu.

Để vẽ lại đồ thị từ dữ liệu đã lưu, chạy `gnuplot plots/figure6.gnuplot` (tương tự cho 7 và 8). Lệnh này ghi đè đồ thị đã lưu trong `plots/`.

### Vẽ ba đồ thị dùng trong báo cáo

Từ thư mục gốc của gói, chạy lệnh dưới đây với Python 3 đã cài Matplotlib.

```bash
python3 plots/make-report-figures.py --output-dir report-figures
```

Script đọc ba bảng `.dat` trong `data/` và xuất `report-figure6.png`, `report-figure7.png` và `report-figure8.png` vào thư mục chỉ định. Đây là các đồ thị ns-3 dùng ở Chương III. Có thể chạy bước này trực tiếp với dữ liệu đã lưu.

## Cấu hình mô hình

| Thành phần | Giá trị |
|---|---|
| Cấu trúc mạng | 15 nút, 14 liên kết, tốc độ mỗi liên kết 10 Mbps |
| Độ trễ | nguồn–R1 2,5 ms; R1–R2 7,5 ms; R2–đích nền 2,5 ms; R2–G 0 ms; G–D0 bằng d = (RTT − 20)/2 ms |
| RTT | luồng nền 25 ms; luồng khảo sát bằng RTT lan truyền mục tiêu |
| Lỗi liên kết | `RateErrorModel` theo gói trên G–D0, cùng PER ở cả hai chiều |
| Hàng đợi R1–R2 | RED, `MinTh` 5, `MaxTh` 15, tối đa 50 gói, không ECN; hàng đợi thiết bị 1 gói |
| TCP | MSS 536 byte, SACK bật, bộ đệm gửi và nhận mỗi đầu 2 MiB, RTT tham chiếu Hybla 25 ms (thuộc tính `TcpHybla::RRTT` trong ns-3) |
| Ứng dụng | `BulkSend` của luồng khảo sát và các luồng nền đang bật bắt đầu ở giây 1; `PacketSink` bắt đầu ở giây 0; dừng ở giây 1 + T |
| Goodput | G = 8 × B<sub>rx</sub> / (T × 10⁶) Mbps, với B<sub>rx</sub> là số byte `PacketSink` nhận được và T là `--duration` tính bằng giây |
| Cấu hình ngẫu nhiên | `RngSeed=1`, `RngRun=1` |

Các tham số TCP khác giữ mặc định của ns-3.49, ví dụ cửa sổ khởi đầu 10 đoạn, ngưỡng Slow Start ban đầu là `UINT32_MAX` byte và ACK trễ.

## Thay đổi so với ns-3 gốc

[`patches/tcp-hybla.patch`](patches/tcp-hybla.patch) sửa hai chỗ trong `tcp-hybla.cc`. `tcp-hybla.h` giữ nguyên.

1. `PktsAcked()` cập nhật ρ mỗi khi RTT nhỏ nhất đã ghi nhận có giá trị dương và hữu hạn. Mã gốc chỉ cập nhật khi mẫu RTT truyền vào `PktsAcked()` bằng đúng RTT nhỏ nhất.
2. `SlowStart()` giới hạn lượng tăng bằng phần còn lại đến `ssthresh` trước khi chuyển sang `uint32_t` để tránh chuyển giá trị vượt miền biểu diễn khi ρ lớn (ví dụ ρ = 24).

## Dữ liệu đã lưu

| Thư mục | Nội dung | Dùng trong báo cáo |
|---|---|---|
| [`data/figure6/`](data/figure6/) | 11 log 600 giây | Bảng III.1, III.2, Hình III.2 |
| [`data/figure7/`](data/figure7/) | 14 log và `summary.tsv` | Bảng III.1, III.3, Hình III.4 |
| [`data/figure8/`](data/figure8/) | 14 log (PER 1% và 5%) và `summary.tsv` | Bảng III.1, III.4, Hình III.6 |
| [`data/ack25/`](data/ack25/) | Hybla và NewReno tại 25 ms với ACK mặc định và `DelAckCount=1`, cùng log ρ | Bảng III.5 |
| [`data/cwnd-startup/`](data/cwnd-startup/) | Trace ρ và cửa sổ lúc khởi động ở 25, 300, 600 ms | Mục II.4.2 |
| [`data/ssthresh600/`](data/ssthresh600/) | Hybla ở 600 ms với ngưỡng mặc định và 750.000 byte | Bảng III.6 |
| [`data/mss1024/`](data/mss1024/) | NewReno của Figure 7 với MSS 1.024 byte ở 25 và 300 ms | Mục III.3.3 |

Các bảng `figure{6,7,8}-goodput.dat` nằm trực tiếp trong `data/`. Figure 6 và 7 có các cột RTT, Hybla, NewReno. Figure 8 có các cột RTT và goodput ở PER 0%, 1%, 5%.

Figure 6 còn ba lượt không có log gốc: Hybla 300 ms, NewReno 300 ms và NewReno 600 ms. Goodput của ba lượt này nằm trong `figure6-goodput.dat` và có ảnh minh chứng 17–18. Các log dài được nén thành `.log.gz`, đọc bằng `zcat`. `summary.tsv` của Figure 7 không có cột `PER_pct` vì mọi lượt đều dùng PER 1%. Bảng do `run-figures.sh` tạo có cột này.

## Ảnh minh chứng

[`screenshots/`](screenshots/) có 35 ảnh. [Bảng tra](screenshots/README.md) ghi lệnh, script hoặc cách tạo và tệp kết quả nếu còn lưu. Ảnh 08–13 ghi giai đoạn thăm dò, trong đó ảnh 08, 12 và 13 chụp từ mã trước khi chỉnh Hybla. Ảnh 16–19 minh chứng các lượt tại 25, 300 và 600 ms. Ảnh 20 ghi đợt quét bốn RTT còn lại.

## Phạm vi

Các bảng Figure 6–8 dùng một lượt mô phỏng cho mỗi cấu hình. Phạm vi gồm các kịch bản Hybla và NewReno có SACK trong bảng đầu trang.

## Giấy phép

Mã TCP Hybla là sửa đổi từ ns-3 nên đi kèm giấy phép `GPL-2.0-only` trong [`COPYING-GPL-2.0`](COPYING-GPL-2.0).
