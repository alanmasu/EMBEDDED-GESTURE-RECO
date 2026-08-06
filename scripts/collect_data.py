import serial
import serial.tools.list_ports
import argparse
import sys
import threading
from pathlib import Path

# ============================================================================
# DATASET CONFIGURATION
# ============================================================================

GESTURES = [
    "jab",
    "hook",
    "overhand",
    "cross",
    "uppercut",
]

SAMPLES_PER_GESTURE = 20

BAUD_RATE = 9600

# ============================================================================

def select_serial_port():
    ports = list(serial.tools.list_ports.comports())

    if not ports:
        print("Error: No USB/Serial devices found.")
        sys.exit(1)

    print("\n--- Available Devices ---")

    for i, port in enumerate(ports):
        print(f"[{i}] {port.device} - {port.description}")

    while True:
        try:
            choice = int(input("\nEnter the number of the port to use: "))

            if 0 <= choice < len(ports):
                return ports[choice].device

            print("Invalid selection.")

        except ValueError:
            print("Please enter a valid integer.")

def collect_single_file(ser, filename):
    with open(filename, "w") as f:

        print(f"\n[CONNECTED] Port: {ser.port}")
        print(f"[SAVING] {filename}")
        print("[INFO] Press ENTER to stop recording.\n")

        stop_event = threading.Event()

        def serial_reader():
            while not stop_event.is_set():

                if ser.in_waiting <= 0:
                    continue

                try:
                    row = ser.readline().decode(
                        "utf-8",
                        errors="replace"
                    ).strip()

                    if not row:
                        continue

                    print(row)
                    f.write(row + "\n")
                    f.flush()

                except Exception as e:
                    print(f"\n[ERROR] {e}")
                    break

        thread = threading.Thread(target=serial_reader, daemon=True)
        thread.start()

        input()

        stop_event.set()
        thread.join()

def collect_dataset(ser, output_dir):
    output_dir.mkdir(parents=True, exist_ok=True)

    print("\n=== DATASET COLLECTION MODE ===")
    print(f"Gestures: {len(GESTURES)}")
    print(f"Samples per gesture: {SAMPLES_PER_GESTURE}")

    total_samples = len(GESTURES) * SAMPLES_PER_GESTURE
    print(f"Total samples: {total_samples}\n")

    for gesture in GESTURES:

        output_file = output_dir / f"{gesture}.csv"

        print("\n" + "=" * 60)
        print(f"Gesture: {gesture}")
        print(f"Target samples: {SAMPLES_PER_GESTURE}")
        input("Press ENTER when ready...")
        print("=" * 60)
        collected = 0

        ser.reset_input_buffer()
        
        with open(output_file, "w") as f:
            ser.write(b"printHeader\n")
            ser.flush()

            while collected < SAMPLES_PER_GESTURE:

                if ser.in_waiting <= 0:
                    continue

                try:
                    row = ser.readline().decode(
                        "utf-8",
                        errors="replace"
                    ).strip()

                    if not row:
                        continue

                    f.write(row + "\n")
                    f.flush()

                    collected += 1

                    print(
                        f"\r{gesture}: "
                        f"{collected}/{SAMPLES_PER_GESTURE}",
                        end="",
                        flush=True
                    )

                except Exception as e:
                    print(f"\n[ERROR] {e}")
                    return

        print(f"\nSaved: {output_file}")

    print("\n=== DATASET COLLECTION COMPLETED ===")

def main():
    parser = argparse.ArgumentParser(
        description="Serial logger / dataset collector"
    )

    parser.add_argument(
        "output",
        help="Output file or output directory"
    )

    args = parser.parse_args()

    output_path = Path(args.output)

    port_name = select_serial_port()

    try:
        ser = serial.Serial(
            port_name,
            BAUD_RATE,
            timeout=1
        )

        if output_path.exists() and output_path.is_dir():
            collect_dataset(ser, output_path)

        elif not output_path.exists():
            if output_path.suffix:
                collect_single_file(ser, output_path)
            else:
                collect_dataset(ser, output_path)

        else:
            collect_single_file(ser, output_path)

    except serial.SerialException as e:
        print(f"\n[SERIAL ERROR] {e}")

    except KeyboardInterrupt:
        print("\n[INTERRUPTED]")

    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

        print("\n[FINISHED]")

if __name__ == "__main__":
    main()