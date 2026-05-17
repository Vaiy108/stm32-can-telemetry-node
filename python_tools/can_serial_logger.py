import csv
import time
from datetime import datetime
import serial


PORT = "COM8"
BAUDRATE = 2_000_000
OUTPUT_FILE = "can_log_decoded.csv"
FRAME_SIZE = 20


def decode_usbcan_frame(frame: bytes):
    if len(frame) != FRAME_SIZE:
        return None

    if frame[0] != 0xAA or frame[1] != 0x55:
        return None

    can_id = (frame[4] << 8) | frame[5]
    dlc = frame[9]
    data = frame[10 : 10 + dlc]

    return can_id, dlc, data


def main():
    print(f"Opening USB-CAN adapter on {PORT} at {BAUDRATE} baud...")

    rx_buffer = bytearray()

    with serial.Serial(PORT, BAUDRATE, timeout=1) as ser, open(
        OUTPUT_FILE, "w", newline=""
    ) as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["timestamp_pc", "elapsed_s", "can_id", "dlc", "data_hex"])

        start_time = time.time()

        print("Decoded CAN logging started. Press Ctrl+C to stop.\n")

        try:
            while True:
                rx_buffer.extend(ser.read(64))

                while len(rx_buffer) >= FRAME_SIZE:
                    start_index = rx_buffer.find(b"\xAA\x55")

                    if start_index < 0:
                        rx_buffer.clear()
                        break

                    if start_index > 0:
                        del rx_buffer[:start_index]

                    if len(rx_buffer) < FRAME_SIZE:
                        break

                    frame = bytes(rx_buffer[:FRAME_SIZE])
                    del rx_buffer[:FRAME_SIZE]

                    decoded = decode_usbcan_frame(frame)

                    if decoded is None:
                        continue

                    can_id, dlc, data = decoded

                    elapsed = time.time() - start_time
                    timestamp = datetime.now().isoformat(timespec="milliseconds")
                    data_hex = " ".join(f"{byte:02X}" for byte in data)

                    print(
                        f"[{elapsed:8.3f}s] "
                        f"CAN ID=0x{can_id:03X} DLC={dlc} DATA={data_hex}"
                    )

                    writer.writerow(
                        [timestamp, f"{elapsed:.3f}", f"0x{can_id:03X}", dlc, data_hex]
                    )
                    csvfile.flush()

        except KeyboardInterrupt:
            print("\nDecoded CAN logging stopped.")


if __name__ == "__main__":
    main()