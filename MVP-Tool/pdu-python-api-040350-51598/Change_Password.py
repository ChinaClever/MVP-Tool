import serial
import time
import sys
import re
import serial.tools.list_ports


def find_usb_serial_port():
    """
    Finds the first available serial port that is listed as a USB Serial Port.
    Returns:
        The COM port name (e.g., 'COM3') if found, None otherwise.
    """
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        if "Pod Master Serial" in port.description or "USB" in port.description:
            print(f"Detected USB Serial Port: {port.device}")
            return port.device
    return None

BAUD_RATE = 115200  # Adjust based on your device

# Find the serial port
COM_PORT = find_usb_serial_port()

if COM_PORT:
    try:
        # Open the serial connection
        ser = serial.Serial(COM_PORT, baudrate=BAUD_RATE, timeout=2)
        print(f"USB Serial Port {COM_PORT} opened successfully.")

        # Close the port when done
    except serial.SerialException as e:
        print(f"Error opening USB Serial Port {COM_PORT}: {e}")

else:
    print("No USB Serial Ports found.")
    
def send_command(ser, command):
    """
    Sends a command over the serial connection and returns the response.
    """
    if not ser or not ser.is_open:
        raise serial.SerialException("Serial port is not open.")
    ser.write((command + '\n').encode())  # Send command
    time.sleep(1)  # Wait for response
    response = ser.readlines()  # Read all available lines
    return [line.decode('utf-8').strip() for line in response]

    
response1 = send_command(ser, "admin")
response2 = send_command(ser, "legrand")
response3 = send_command(ser, "Rar!tan0")
response4 = send_command(ser, "Rar!tan0")
