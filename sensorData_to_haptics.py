import pandas as pd


COM_PORT = 'COM7'   #Adjust if needed
BAUD_RATE = 115200
CSV_FILE = "sync_test2509.csv"
VIDEO_FILE = "sync_test2509.mp4"


def mag_to_strength(mag):
    """Map magnitude to Low/Med/High vibration strength."""
    if mag >= 3.0:
        return "High"
    elif mag >= 2.0:
        return "Med"
    else:
        return "Low"
