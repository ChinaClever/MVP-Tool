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
        print(f"USB 串口 {COM_PORT} 打开成功 .")

        # Close the port when done
    except serial.SerialException as e:
        print(f"USB 串口 {COM_PORT} 打开失败 : {e}")

else:
    print("找不到 USB 串口.")
    
def send_command(ser, command):
    """
    Sends a command over the serial connection and returns the response.
    """
    if not ser or not ser.is_open:
        raise serial.SerialException("串口 关闭.")
    ser.write((command + '\n').encode())  # Send command
    time.sleep(1)  # Wait for response
    response = ser.readlines()  # Read all available lines
    return [line.decode('utf-8').strip() for line in response]

# Example usage
response = send_command(ser, "")

# Join the list of response lines into a single string for regex search
combined_response = ' '.join(response)

# Regex to find all IP addresses starting with 192.168.
#ip_pattern = r'192\.168\.\d{1,3}\.\d{1,3}'
ip_pattern = r'169\.254\.\d{1,3}\.\d{1,3}'

matches = re.findall(ip_pattern, combined_response)

# Print the matched IPs

#agent = rpc.Agent("https", matches[0], "admin", "Rar!tan0", disable_certificate_verification=True)
agent = rpc.Agent("https", matches[0], "admin", "LegrandfaTUV0", disable_certificate_verification=True)
pdu = pdumodel.Pdu("/model/pdu/0", agent)

metadata = pdu.getMetaData()

serial_number = getattr(metadata, 'nameplate', None)
serial_number = getattr(serial_number, 'serialNumber', 'N/A')
hw_revision = getattr(metadata, 'hwRevision', 'N/A')
fw_revision = getattr(metadata, 'fwRevision', 'N/A')
# mac_address = getattr(metadata, 'macAddress', 'N/A')

# 仅输出结构化字段（无标题、无注释、无调试信息）
print(f"接口信息输出：")
print(f"SerialNumber={serial_number}")
print(f"HwRevision={hw_revision}")
print(f"FwRevision={fw_revision}")
#裝置資訊

pdu0 = pdumodel.Pdu("/model/pdu/0", agent)
pdu0.getBeeper().activate(True, "", 10)
Beeper_result = False
for n in range(0, 3):
    Beeper_test = input("你听到蜂鸣声了吗？ (y/n)")
    if Beeper_test.upper() == "Y":
        print("1.蜂鸣器 测试 通过")
        Beeper_result = True
        break
    elif Beeper_test.upper() == "N":
        print(" 重新 测试 ")
    else:
        print("非法输入, 请按下 'Y' or 'N'")
if Beeper_result == False:
    print("1.蜂鸣器 测试 失败")
    
pdm = raritan.rpc.peripheral.DeviceManager("/model/peripheraldevicemanager", agent)

slots = pdm.getDeviceSlots()
Detected_Sensor = False
for num, slot in enumerate(slots):
    settings = slot.getSettings()
    device = slot.getDevice()
    if device == None:
        continue
    else:
#        print("Slot %d: %s (%s)" % (num + 1, settings.name, device.deviceID.serial))
        if device.device:
            if device.deviceID.type.readingtype == raritan.rpc.sensors.Sensor.NUMERIC:
                reading = device.device.getReading()
                if reading.value != 0:
#                    print("  Reading: %f" % reading.value)
                    Detected_Sensor = True
            else:
                state = device.device.getState()
#                print("  State: %d" % state.value)

if Detected_Sensor:
    print("2.传感器 测试 通过")
else:
    print("2.传感器 测试 失败")
    
# USB Proxy to fetch USB devices information
usb_proxy = usb.Usb("/usb", agent)
usb_numbers = usb_proxy.getHostPortCount()
usb_info = usb_proxy.getDevices()
usb_devices_count = 0
for device in usb_info:
    usb_devices_count += 1  # Increment the USB devices count

# Check if 2 USB devices and sensor data are found
if usb_devices_count == 2:
    print("3.USB 测试 通过")
else:
    print("3.USB 测试 失败")


debuglog = logging.DebugLog("/debuglog", agent)  
output = debuglog.getChunk(0, 1000, logging.RangeDirection.FORWARD)
RTC = ["rtc-rv3028", "registered as rtc0", "setting system clock to"]
Bluetooth = ["hci0"]

def search_log_entries(log_chunk, keywords):
    matched = []
    for entry in log_chunk.selEntries:
        if not entry or not entry.message or not entry.timestamp:
            continue  # Skip incomplete entries
        message = entry.message.lower()
        if any(keyword in message for keyword in keywords):
            log_line = f"[{entry.timestamp}] {entry.message}"
            matched.append(log_line)
    return matched

    
def verify_RTC_log(logs):
    """
    Verifies if all expected RTC patterns are present in the log string.
    """
    boot_reason_pattern = r"\[\s*\d+\.\d+\] Boot reason\(s\) 0x00000000: Power-on or pin-hole reset"
    rtc_vbackup_pattern = r"\[\s*\d+\.\d+\] rtc-rv3028 \d+-\d+: RTC was on Vbackup: Controller was powered off for a short time"
    rtc_registered_pattern = r"\[\s*\d+\.\d+\] rtc-rv3028 \d+-\d+: registered as rtc0"
    rtc_system_clock_pattern = r"\[\s*\d+\.\d+\] rtc-rv3028 \d+-\d+: setting system clock to \d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2} UTC \(\d+\)"

    return all(
        re.search(pattern, logs)
        for pattern in [
            boot_reason_pattern,
            rtc_vbackup_pattern,
            rtc_registered_pattern,
            rtc_system_clock_pattern
        ]
    )
RTC_Logs = search_log_entries(output, RTC)
rtc_log_string = "\n".join(RTC_Logs)
rtc_verified = verify_RTC_log(rtc_log_string)
if search_log_entries(output, RTC):
    print("4.RTC 测试 通过")
else:
    print("4.RTC 测试 失败")

pattern = r"""
    ^\[              # Opening square bracket
    \d{4}-\d{2}-\d{2}  # Date (e.g., 2000-01-03)
    \s
    \d{2}:\d{2}:\d{2}  # Time (e.g., 19:23:12)
    \]\s\[INF\]:\[kernel\]:\[.*\]  # Remaining log format
"""
def check_bluetooth_output(result):
    if isinstance(result, list):
        result = "\n".join(filter(None, result))  # Remove any None entries
    if not result:
        return True  # No data means invalid
    lines = result.strip().splitlines()
    compiled_pattern = re.compile(pattern, re.VERBOSE)
    for line in lines:
        if not line or not compiled_pattern.match(line.strip()):
            return True  # At least one line doesn't match
    return False

bluetooth_logs = search_log_entries(output, Bluetooth)
has_invalid_bluetooth_output = check_bluetooth_output(bluetooth_logs)
if has_invalid_bluetooth_output:
    print("5.蓝牙 测试 失败")
else:
    print("5.蓝牙 测试 成功")

#Ethernet test
#Interfaces to check
info = net.Net("/net", agent).getInfo()
interfaces = ["eth0", "eth1", "eth2"]

all_passed = True  # Assume success until proven otherwise

for iface in interfaces:
    try:
        iface_info = info.ifMap[iface]
        ipv4_addrs = iface_info.ipv4.addrsCidr

        if not ipv4_addrs:
            all_passed = False
            break

        ip = ipv4_addrs[0].addr  # Use the first IP
        if not ip.startswith("192.254."):
            all_passed = False
            break

    except (KeyError, AttributeError):
        all_passed = False
        break

# Final result
if all_passed:
    print("6.ETHERNET 测试 通过")
else:
    print("6.ETHERNET 测试 失败")


#Zigbee Test
reading_output = pdu.getPeripheralDeviceManager().getDeviceSlot(0).getDevice().device.getReading()
#yprint(reading_output)
# Convert the output to a string if it's not already
reading_str = str(reading_output)
# Check if both flags are True
available_true = "* available = True" in reading_str
valid_true = "* valid     = True" in reading_str

if available_true and valid_true:
    print("7.ZIGBEE 测试 通过")
else:
    print("7.ZIGBEE 测试 失败")