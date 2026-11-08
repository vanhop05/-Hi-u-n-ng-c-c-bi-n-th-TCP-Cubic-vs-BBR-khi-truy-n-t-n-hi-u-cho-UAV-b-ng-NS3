import subprocess
import os
import shutil
import pandas as pd
import matplotlib.pyplot as plt

plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
plt.rcParams['font.sans-serif'] = 'DejaVu Sans'

BASE_DIR = os.path.expanduser('~/ns3-workspace/ns-3.49')
OUT_WIN_DIR = '/mnt/d/Lập Trình Mạng/ket_qua_uav'
os.makedirs(OUT_WIN_DIR, exist_ok=True)

print("=== 1. CHAY MO PHONG UAV KHONG DAY (CUBIC VS BBR) ===")
for prot in ['TcpCubic', 'TcpBbr']:
    print(f"  -> Dang mo phong Drone bay voi {prot}...")
    cmd = (
        f"./ns3 run 'scratch/uav-wifi-tcp "
        f"--transportProt={prot} "
        f"--uavSpeed=10.0 "
        f"--simTime=40.0 "
        f"--altitude=25.0 "
        f"--prefix=uav_eval'"
    )
    subprocess.run(cmd, shell=True, cwd=BASE_DIR, check=True)

# Duong dan file CSV sinh ra trong thu muc NS-3
cubic_file = os.path.join(BASE_DIR, "uav_eval_TcpCubic_metrics.csv")
bbr_file = os.path.join(BASE_DIR, "uav_eval_TcpBbr_metrics.csv")

# Copy sang thu muc Windows
shutil.copy(cubic_file, os.path.join(OUT_WIN_DIR, "uav_TcpCubic_metrics.csv"))
shutil.copy(bbr_file, os.path.join(OUT_WIN_DIR, "uav_TcpBbr_metrics.csv"))

df_cubic = pd.read_csv(cubic_file)
df_bbr = pd.read_csv(bbr_file)

print("\n=== 2. VE BIEU DO KHAO SAT KHOANG CACH BAY UAV VS THONG LUONG ===")

# --- HINH 1: Khoang cach bay vs Thong luong (Throughput vs Distance) ---
plt.figure(figsize=(9, 5.5), dpi=150)
plt.plot(df_cubic['Distance_m'], df_cubic['Throughput_Mbps'], label='TCP Cubic (Truyen thong)', color='#d62728', linewidth=2.2, alpha=0.85)
plt.plot(df_bbr['Distance_m'], df_bbr['Throughput_Mbps'], label='TCP BBR (Google)', color='#1f77b4', linewidth=2.2, alpha=0.85)
plt.axhline(y=10.0, color='#888888', linestyle='--', label='Nguong Video HD (10 Mbps)')
plt.title("Anh huong cua Khoang cach bay UAV den Thong luong Video\n(Do cao bay = 25m, Song Wi-Fi 802.11a, Suy hao khong gian)", fontsize=12, pad=12)
plt.xlabel("Khoang cach giua UAV va Tram mat dat GCS (met)", fontsize=11)
plt.ylabel("Thong luong truyen Video (Mbps)", fontsize=11)
plt.legend(fontsize=10, frameon=True)
plt.tight_layout()
fig1_path = os.path.join(OUT_WIN_DIR, "1_uav_khoangcach_vs_thongluong.png")
plt.savefig(fig1_path)
plt.close()

# --- HINH 2: Thoi gian bay vs Cua so nghen CWND ---
plt.figure(figsize=(9, 5.5), dpi=150)
plt.plot(df_cubic['Time'], df_cubic['Cwnd_Packets'], label='TCP Cubic (CWND)', color='#d62728', linewidth=1.5, alpha=0.8)
plt.plot(df_bbr['Time'], df_bbr['Cwnd_Packets'], label='TCP BBR (CWND)', color='#1f77b4', linewidth=1.5, alpha=0.8)
plt.title("Dac tuyen Cua so nghen (CWND) khi UAV bay ra xa theo thoi gian", fontsize=12, pad=12)
plt.xlabel("Thoi gian bay (giay)", fontsize=11)
plt.ylabel("Cua so nghen CWND (so goi tin)", fontsize=11)
plt.legend(fontsize=10, frameon=True)
plt.tight_layout()
fig2_path = os.path.join(OUT_WIN_DIR, "2_uav_dactuyen_cwnd.png")
plt.savefig(fig2_path)
plt.close()

print("\n=== DA VE XONG TOAN BO BIEU DO UAV! ===")
print(f"Thu muc ket qua UAV: D:\\Lập Trình Mạng\\ket_qua_uav")
