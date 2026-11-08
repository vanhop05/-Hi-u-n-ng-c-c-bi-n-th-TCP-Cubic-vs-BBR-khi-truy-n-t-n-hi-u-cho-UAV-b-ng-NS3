import subprocess
import os
import re
import pandas as pd
import matplotlib.pyplot as plt

# Thiết lập style đồ họa
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
plt.rcParams['font.sans-serif'] = 'DejaVu Sans'
plt.rcParams['axes.edgecolor'] = '#cccccc'
plt.rcParams['axes.linewidth'] = 0.8

BASE_DIR = os.path.expanduser('~/ns3-workspace/ns-3.49')
RESULTS_DIR = os.path.join(BASE_DIR, 'results_experiment')
OUT_WIN_DIR = '/mnt/d/Lập Trình Mạng/ket_qua_do_an'

os.makedirs(RESULTS_DIR, exist_ok=True)
os.makedirs(OUT_WIN_DIR, exist_ok=True)

def run_sim(prot, bw, delay, loss, sim_time=15.0, prefix="sim"):
    cmd = (
        f"./ns3 run 'scratch/tcp-comparison "
        f"--transportProt={prot} "
        f"--bandwidth={bw} "
        f"--delay={delay} "
        f"--errorRate={loss} "
        f"--simTime={sim_time} "
        f"--prefix={RESULTS_DIR}/{prefix}'"
    )
    res = subprocess.run(cmd, shell=True, cwd=BASE_DIR, capture_output=True, text=True)
    for line in res.stdout.split('\n'):
        if line.startswith("SUMMARY,"):
            parts = line.strip().split(',')
            return {
                "protocol": parts[1],
                "bandwidth": parts[2],
                "delay": parts[3],
                "loss": float(parts[4]),
                "total_bytes": int(parts[5]),
                "throughput_mbps": float(parts[6])
            }
    return None

print("=== 1. CHAY THUC NGHIEM: ANH HUONG CUA TY LE MAT GOI (PACKET LOSS) ===")
loss_rates = [0.0, 0.005, 0.01, 0.02, 0.05]
loss_data = []
for loss in loss_rates:
    for prot in ['TcpCubic', 'TcpBbr']:
        print(f"  -> Dang chay {prot} voi Packet Loss = {loss * 100}%...")
        res = run_sim(prot, "10Mbps", "20ms", loss, sim_time=15.0, prefix=f"loss_{loss}_{prot}")
        if res:
            loss_data.append(res)

df_loss = pd.DataFrame(loss_data)

print("\n=== 2. CHAY THUC NGHIEM: ANH HUONG CUA DO TRE (DELAY / RTT) ===")
delays = ["10ms", "30ms", "60ms", "100ms", "150ms"]
delay_data = []
for d in delays:
    for prot in ['TcpCubic', 'TcpBbr']:
        print(f"  -> Dang chay {prot} voi Delay = {d} (Loss = 1%)...")
        res = run_sim(prot, "10Mbps", d, 0.01, sim_time=15.0, prefix=f"delay_{d}_{prot}")
        if res:
            delay_data.append(res)

df_delay = pd.DataFrame(delay_data)

print("\n=== 3. CHAY THUC NGHIEM: ANH HUONG CUA BANG THONG (BANDWIDTH) ===")
bandwidths = ["5Mbps", "10Mbps", "20Mbps", "40Mbps"]
bw_data = []
for bw in bandwidths:
    for prot in ['TcpCubic', 'TcpBbr']:
        print(f"  -> Dang chay {prot} voi Bandwidth = {bw} (Loss = 1%)...")
        res = run_sim(prot, bw, "20ms", 0.01, sim_time=15.0, prefix=f"bw_{bw}_{prot}")
        if res:
            bw_data.append(res)

df_bw = pd.DataFrame(bw_data)

print("\n=== 4. CHAY CHI TIET MOT KICH BAN DE VE DONG HOC CWND & THROUGHPUT THEO THOI GIAN ===")
print("  -> Chay Cubic (10Mbps, 20ms, 1% loss)...")
run_sim("TcpCubic", "10Mbps", "20ms", 0.01, sim_time=20.0, prefix="timeseries")
print("  -> Chay BBR (10Mbps, 20ms, 1% loss)...")
run_sim("TcpBbr", "10Mbps", "20ms", 0.01, sim_time=20.0, prefix="timeseries")

print("\n=== DANG VE CAC BIEU DO DANH GIA DANG CAP BAO CAO ===")

# --- HINH 1: Packet Loss vs Throughput ---
plt.figure(figsize=(8, 5), dpi=150)
for prot, color, marker in [('TcpCubic', '#d62728', 'o'), ('TcpBbr', '#1f77b4', 's')]:
    sub = df_loss[df_loss['protocol'] == prot]
    plt.plot(sub['loss'] * 100, sub['throughput_mbps'], label=prot, color=color, marker=marker, linewidth=2.2, markersize=7)
plt.title("Anh huong cua Ty le mat goi (Packet Loss) den Thong luong\n(Bang thong = 10Mbps, Do tre = 20ms)", fontsize=12, pad=12)
plt.xlabel("Ty le mat goi (%)", fontsize=11)
plt.ylabel("Thong luong trung binh (Mbps)", fontsize=11)
plt.ylim(0, 11)
plt.legend(fontsize=11, frameon=True)
plt.tight_layout()
fig1_path = os.path.join(OUT_WIN_DIR, "1_anh_huong_mat_goi.png")
plt.savefig(fig1_path)
plt.close()

# --- HINH 2: Delay vs Throughput ---
plt.figure(figsize=(8, 5), dpi=150)
delay_nums = [int(d.replace('ms', '')) for d in delays]
for prot, color, marker in [('TcpCubic', '#d62728', 'o'), ('TcpBbr', '#1f77b4', 's')]:
    sub = df_delay[df_delay['protocol'] == prot]
    plt.plot(delay_nums, sub['throughput_mbps'], label=prot, color=color, marker=marker, linewidth=2.2, markersize=7)
plt.title("Anh huong cua Do tre (Delay) den Thong luong\n(Bang thong = 10Mbps, Mat goi = 1%)", fontsize=12, pad=12)
plt.xlabel("Do tre kenh truyen 1 chieu (ms)", fontsize=11)
plt.ylabel("Thong luong trung binh (Mbps)", fontsize=11)
plt.ylim(0, 11)
plt.legend(fontsize=11, frameon=True)
plt.tight_layout()
fig2_path = os.path.join(OUT_WIN_DIR, "2_anh_huong_do_tre.png")
plt.savefig(fig2_path)
plt.close()

# --- HINH 3: Bandwidth vs Throughput ---
plt.figure(figsize=(8, 5), dpi=150)
bw_nums = [int(b.replace('Mbps', '')) for b in bandwidths]
for prot, color, marker in [('TcpCubic', '#d62728', 'o'), ('TcpBbr', '#1f77b4', 's')]:
    sub = df_bw[df_bw['protocol'] == prot]
    plt.plot(bw_nums, sub['throughput_mbps'], label=prot, color=color, marker=marker, linewidth=2.2, markersize=7)
plt.plot(bw_nums, bw_nums, '--', color='#888888', label='Bang thong toi da ly thuyet')
plt.title("Anh huong cua Bang thong den Thong luong thuc te\n(Do tre = 20ms, Mat goi = 1%)", fontsize=12, pad=12)
plt.xlabel("Bang thong cap phat (Mbps)", fontsize=11)
plt.ylabel("Thong luong trung binh (Mbps)", fontsize=11)
plt.legend(fontsize=11, frameon=True)
plt.tight_layout()
fig3_path = os.path.join(OUT_WIN_DIR, "3_anh_huong_bang_thong.png")
plt.savefig(fig3_path)
plt.close()

# --- HINH 4: CWND Theo Thoi Gian (Dac tuyen co che dieu khien nghen) ---
plt.figure(figsize=(10, 5), dpi=150)
cubic_cwnd_file = os.path.join(RESULTS_DIR, "timeseries_TcpCubic_cwnd.csv")
bbr_cwnd_file = os.path.join(RESULTS_DIR, "timeseries_TcpBbr_cwnd.csv")

if os.path.exists(cubic_cwnd_file):
    df_ccwnd = pd.read_csv(cubic_cwnd_file)
    plt.plot(df_ccwnd['Time'], df_ccwnd['Cwnd_Packets'], label='TCP Cubic (Cwnd)', color='#d62728', alpha=0.8, linewidth=1.2)

if os.path.exists(bbr_cwnd_file):
    df_bcwnd = pd.read_csv(bbr_cwnd_file)
    plt.plot(df_bcwnd['Time'], df_bcwnd['Cwnd_Packets'], label='TCP BBR (Cwnd)', color='#1f77b4', alpha=0.8, linewidth=1.2)

plt.title("Dac tuyen Cua so nghen (CWND) theo thoi gian\n(Bang thong = 10Mbps, Do tre = 20ms, Mat goi = 1%)", fontsize=12, pad=12)
plt.xlabel("Thoi gian mo phong (giay)", fontsize=11)
plt.ylabel("Cua so nghen CWND (so goi tin MSS)", fontsize=11)
plt.legend(fontsize=11, frameon=True)
plt.tight_layout()
fig4_path = os.path.join(OUT_WIN_DIR, "4_dactuyen_cwnd_theothoidiem.png")
plt.savefig(fig4_path)
plt.close()

# --- HINH 5: Throughput Theo Thoi Gian ---
plt.figure(figsize=(10, 5), dpi=150)
cubic_tp_file = os.path.join(RESULTS_DIR, "timeseries_TcpCubic_throughput.csv")
bbr_tp_file = os.path.join(RESULTS_DIR, "timeseries_TcpBbr_throughput.csv")

if os.path.exists(cubic_tp_file):
    df_ctp = pd.read_csv(cubic_tp_file)
    plt.plot(df_ctp['Time'], df_ctp['Throughput_Mbps'], label='TCP Cubic', color='#d62728', alpha=0.75, linewidth=1.5)

if os.path.exists(bbr_tp_file):
    df_btp = pd.read_csv(bbr_tp_file)
    plt.plot(df_btp['Time'], df_btp['Throughput_Mbps'], label='TCP BBR', color='#1f77b4', alpha=0.75, linewidth=1.5)

plt.axhline(y=10.0, color='gray', linestyle='--', label='Gioi han 10Mbps')
plt.title("Thong luong tuc thoi theo thoi gian\n(Bang thong = 10Mbps, Do tre = 20ms, Mat goi = 1%)", fontsize=12, pad=12)
plt.xlabel("Thoi gian mo phong (giay)", fontsize=11)
plt.ylabel("Thong luong (Mbps)", fontsize=11)
plt.ylim(0, 13)
plt.legend(fontsize=11, frameon=True)
plt.tight_layout()
fig5_path = os.path.join(OUT_WIN_DIR, "5_dactuyen_throughput_theothoidiem.png")
plt.savefig(fig5_path)
plt.close()

# Lưu bảng tổng hợp kết quả dạng CSV
all_results = pd.concat([df_loss, df_delay, df_bw]).drop_duplicates()
all_results.to_csv(os.path.join(OUT_WIN_DIR, "bang_tong_hop_ket_qua.csv"), index=False)

print("\n=== DA HOAN TAT TOAN BO KHAO SAT VA XUAT FILE THANH CONG ===")
print(f"Thu muc luu ket qua tren Windows: D:\\Lập Trình Mạng\\ket_qua_do_an")
