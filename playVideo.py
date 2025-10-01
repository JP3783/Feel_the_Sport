import os
import time

video_path = "sync_test.mp4"

# Countdown from 5
for i in range(5, 0, -1):
    print(f"Starting in {i}...")
    time.sleep(1)

# Open the video after countdown
os.startfile(video_path)  # Works on Windows
