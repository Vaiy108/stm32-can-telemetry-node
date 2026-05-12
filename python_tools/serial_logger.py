import csv
import time
from datetime import datetime
import serial


PORT = "COM7"
BAUDRATE = 115200
OUTPUT_FILE = "telemetry_log.csv"


def main():
    print(f"Opening {PORT} at {BAUDRATE} baud...")

    with serial.Serial(PORT, BAUDRATE, timeout=1) as ser, open(
        OUTPUT_FILE, "w", newline=""
    ) as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["timestamp_pc", "elapsed_s", "uart_line"])

        start_time = time.time()

        print("Logging started. Press Ctrl+C to stop.\n")

        try:
            while True:
                line = ser.readline().decode(errors="replace").strip()

                if line:
                    elapsed = time.time() - start_time
                    timestamp = datetime.now().isoformat(timespec="milliseconds")

                    print(f"[{elapsed:8.3f}s] {line}")
                    writer.writerow([timestamp, f"{elapsed:.3f}", line])
                    csvfile.flush()

        except KeyboardInterrupt:
            print("\nLogging stopped.")


if __name__ == "__main__":
    main()