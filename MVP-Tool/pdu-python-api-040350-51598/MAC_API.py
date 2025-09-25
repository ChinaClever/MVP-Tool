import serial
import time
import sys
import re
import serial.tools.list_ports
from raritan import rpc
from raritan.rpc import pdumodel
import raritan.rpc.peripheral
from raritan.rpc import usb
from raritan.rpc import zigbee
from raritan.rpc import usermgmt
from raritan.rpc import security
from raritan.rpc import cert
from raritan.rpc import logging
from raritan.rpc import net

def find_usb_serial_port():
    """
    Finds the first available serial port that is listed as a USB Serial Port.
    Returns:
        The COM port name (e.g., 'COM3') if found, None otherwise.
    """
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        if "USB Serial Port" in port.description or "USB" or "Pod Master" in port.description:
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

# Example usage
response = send_command(ser, "")

# Join the list of response lines into a single string for regex search
combined_response = ' '.join(response)

# Regex to find all IP addresses starting with 192.168.
ip_pattern = r'192\.168\.\d{1,3}\.\d{1,3}'
matches = re.findall(ip_pattern, combined_response)

# Print the matched IPs

#agent = rpc.Agent("https", matches[0], "admin", "Rar!tan0", disable_certificate_verification=True)
agent = rpc.Agent("https", matches[0], "admin", "Legrand4TUV", disable_certificate_verification=True)

pdu = pdumodel.Pdu("/model/pdu/0", agent)
#裝置資訊

pdu0 = pdumodel.Pdu("/model/pdu/0", agent)

net_proxy = net.Net("/net", agent)
for label, ifinfo in net_proxy.getInfo().ifMap.items():
    print(label, ifinfo.macAddr)