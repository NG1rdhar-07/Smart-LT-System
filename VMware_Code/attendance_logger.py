import requests
import time
import csv
from datetime import datetime

# Replace with your Blynk Auth Token
BLYNK_AUTH_TOKEN = " # Put your token here :)...its not safe to share !! "
V9_PIN = "V9"  # Pin used for roll number input

# Log file
CSV_FILE = "attendance_log.csv"  	#  databse to store attendence record

# Keep track of last received roll number to avoid duplicates
last_roll = None

def log_attendance(roll_number):
    now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    with open(CSV_FILE, mode='a', newline='') as file:
        writer = csv.writer(file)
        writer.writerow([roll_number, now])
    print(f"✅ Logged: {roll_number} at {now}")

def get_roll_number():
    try:
        url = f"https://blynk.cloud/external/api/get?token={BLYNK_AUTH_TOKEN}&{V9_PIN}"
        response = requests.get(url)
        if response.status_code == 200:
            roll = response.text.strip()
            return roll
    except Exception as e:
        print("Error:", e)
    return None

print("📡 Starting attendance logger...")

while True:
    roll_number = get_roll_number()

    if roll_number and roll_number != "0" and roll_number != last_roll:
        log_attendance(roll_number)
        last_roll = roll_number

    time.sleep(3)  # Check every 3 seconds
