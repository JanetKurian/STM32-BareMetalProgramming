# import can

# bus = can.Bus(
#     # interface="...",
#     # channel="...",
#     bitrate=500000
# )

# msg = can.Message(
#     arbitration_id=0x7E0,
#     data=[0x3E, 0x00],
#     is_extended_id=False
# )

# bus.send(msg)

# response = bus.recv(1.0)

# if response:
#     print(response.arbitration_id, response.data.hex())

import serial
import time

PORT = "COM17"
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)

def send_can(can_id, data):
    # Format expected by STM32:
    # 7E0 02 3E 00
    command = f"{can_id:03X} " + " ".join(f"{b:02X}" for b in data) + "\r\n"

    print(f"\nTX: {command.strip()}")
    ser.write(command.encode("ascii"))

    time.sleep(0.5)

    response = ser.read_all()

    if response:
        print("STM32:")
        print(response.decode("ascii", errors="replace"))
    else:
        print("No UART output")


print("=== UDS TEST ===")

# Tester Present
send_can(
    0x7E0,
    [0x02, 0x3E, 0x00]
)

# Tester Present with response suppression
send_can(
    0x7E0,
    [0x02, 0x3E, 0x80]
)

# Read Data By Identifier
send_can(
    0x7E0,
    [0x03, 0x22, 0xF1, 0x90]
)

# Write Data By Identifier
send_can(
    0x7E0,
    [0x03, 0x2E, 0xF1, 0x90]
)

ser.close()