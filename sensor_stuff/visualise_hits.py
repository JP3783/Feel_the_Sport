import pandas as pd
import matplotlib.pyplot as plt
import sys
from datetime import datetime

#GET CSV FILE FROM ARGUMENT
if len(sys.argv) < 2:
    print("Usage: python visualize_hits.py <csv_filename>")
    sys.exit(1)

CSV_FILENAME = sys.argv[1]
OUTPUT_PREFIX = CSV_FILENAME.rsplit(".", 1)[0]  #Prefix from file name
SAVE_PLOTS = True

#LOAD DATA
try:
    df = pd.read_csv(CSV_FILENAME)
except FileNotFoundError:
    print(f"File not found: {CSV_FILENAME}")
    sys.exit(1)

#Convert Time column to datetime if possible
try:
    df["Time"] = pd.to_datetime(df["Time"])
except Exception:
    print("Time column not parsed as datetime — using raw values.")

#PLOT 1: Magnitude over Time with Hits
plt.figure(figsize=(10, 5))
plt.plot(df["Time"], df["Mag"], label="Magnitude", color="blue", linewidth=1)

#Highlight hits if Notes column exists
if "Notes" in df.columns:
    hits = df[df["Notes"] == "HIT"]
    if not hits.empty:
        plt.scatter(hits["Time"], hits["Mag"], color="red", label="Hits", zorder=5)

plt.xlabel("Time")
plt.ylabel("Magnitude (m/s²)")
plt.title("Tennis Hit Magnitude Over Time")
plt.grid(True)
plt.legend()
plt.tight_layout()

if SAVE_PLOTS:
    plt.savefig(f"{OUTPUT_PREFIX}_magnitude.png", dpi=300)

plt.show()

#PLOT 2: X, Y, Z Raw Accelerometer Data
plt.figure(figsize=(10, 5))
plt.plot(df["Time"], df["X"], label="X-axis", color="red", linewidth=1)
plt.plot(df["Time"], df["Y"], label="Y-axis", color="green", linewidth=1)
plt.plot(df["Time"], df["Z"], label="Z-axis", color="blue", linewidth=1)

plt.xlabel("Time")
plt.ylabel("Acceleration (m/s²)")
plt.title("Raw Accelerometer Data")
plt.grid(True)
plt.legend()
plt.tight_layout()

if SAVE_PLOTS:
    plt.savefig(f"{OUTPUT_PREFIX}_raw_axes.png", dpi=300)

plt.show()

print("Plots generated successfully!")
if SAVE_PLOTS:
    print(f"Saved as '{OUTPUT_PREFIX}_magnitude.png' and '{OUTPUT_PREFIX}_raw_axes.png'")
