import serial
import time
import sys
import re
import subprocess
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

def ping_host(host="podmaster.local", timeout=2):
    """
    Ping 指定主机，返回 True 表示 ping 通，False 表示失败
    """
    try:
        # Windows 下 ping 1 次
        if sys.platform.startswith("win"):
            cmd = ["ping", host, "-n", "1", "-w", str(timeout*1000)]
        else:
            # Linux / macOS
            cmd = ["ping", "-c", "1", "-W", str(timeout), host]

        result = subprocess.run(cmd, capture_output=True, text=True)
        output = result.stdout

        # 查找 IP 是否以 192.168 开头
        import re
        match = re.search(r"(192\.168\.\d{1,3}\.\d{1,3})", output)
        if match:
            print(f" {host} 可达, IP={match.group(1)}")
            return True
        else:
            print(f" {host} 不可达或 IP 不合法")
            return False

    except Exception as e:
        print(f" Ping 出错: {e}")
        return False

# 调用 ping，失败就退出
if not ping_host("podmaster.local"):
    sys.exit(1)

def find_usb_serial_port():
    """
    Finds the first available serial port that is listed as a USB Serial Port.
    Returns:
        The COM port name (e.g., 'COM3') if found, None otherwise.
    """
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        if "USB Serial Port" in port.description or "USB" in port.description:
            print(f"检测到 USB 串口 : {port.device}")
            return port.device
    return None

BAUD_RATE = 115200  # Adjust based on your device

# Find the serial port
COM_PORT = find_usb_serial_port()

if COM_PORT:
    try:
        # Open the serial connection
        ser = serial.Serial(COM_PORT, baudrate=BAUD_RATE, timeout=2)
        print(f" USB 串口{COM_PORT} 成功打开.")

        # Close the port when done
    except serial.SerialException as e:
        print(f"USB 串口 {COM_PORT} 打开失败: {e}")

else:
    print("没有找到 USB 串口.")
    
def send_command(ser, command):
    """
    Sends a command over the serial connection and returns the response.
    """
    if not ser or not ser.is_open:
        raise serial.SerialException("USB 串口 没有打开.")
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

agent = rpc.Agent("https", matches[0], "admin", "Legrand4TUV", disable_certificate_verification=True)
system_cfg = agent.get("/cgi-bin/system_cfg.cgi").decode("utf-8")
for line in system_cfg.split():
    key, value = line.split("=", 1)
    if key == "ZIGBEE_MAC":
        print(f"ZB: {value}")
    elif key == "BLUETOOTH_MAC":
        print(f"BT: {value}")
    elif key == "BOARD_SERIAL":
        print(f"BOARD SERIAL: {value}")

pdu = pdumodel.Pdu("/model/pdu/0", agent)
#裝置資訊
pdm = raritan.rpc.peripheral.DeviceManager("/model/peripheraldevicemanager", agent)

pdu0 = pdumodel.Pdu("/model/pdu/0", agent)
pdu0.getBeeper().activate(True, "", 10)
Beeper_result = False
for n in range(0, 3):
    Beeper_test = input("Did you hear the beep? Y/N: ")
    if Beeper_test.upper() == "Y":
        print("1.蜂鸣器 测试 通过")
        Beeper_result = True
        break
    elif Beeper_test.upper() == "N":
        print("重试 蜂鸣器 测试")
    else:
        print("非法输入 'Y' or 'N'")
if Beeper_result == False:
    print("1.蜂鸣器 测试 失败")

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
#print(usb_info)
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
#print(RTC_Logs)
rtc_log_string = "\n".join(RTC_Logs)
rtc_verified = verify_RTC_log(rtc_log_string)
if search_log_entries(output, RTC):
    print("4.RTC 测试 通过")
else:
    print("4.RTC 测试 失败")

'''pattern = r"""
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
#print(bluetooth_logs)

has_invalid_bluetooth_output = check_bluetooth_output(bluetooth_logs)
if has_invalid_bluetooth_output:
    print("5.蓝牙 测试 失败")
else:
    print("5.蓝牙 测试 通过")'''

#Secure Element Test
Secure_Element = security.Security("/security", agent).getTpmInfo()

def check_tpm_detected(tpm_info):
    try:
        return tpm_info.detected is True
    except AttributeError:
        return False
if check_tpm_detected(Secure_Element):
    print("5.调试口关闭 测试 通过")
else:
    print("5.调试口关闭 测试 失败")
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
            continue


        ip = ipv4_addrs[0].addr  # Use the first IP
        print(ip)
        if not ip.startswith("192.168."):
            all_passed = False



    except (KeyError, AttributeError):
        all_passed = False
        break

# Final result
if all_passed:
    print("6.网口 测试 通过")
else:
    print("6.网口 测试 失败")



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

pdu = pdumodel.Pdu("/model/pdu/0", agent)
#裝置資訊

pdu0 = pdumodel.Pdu("/model/pdu/0", agent)

net_proxy = net.Net("/net", agent)
for label, ifinfo in net_proxy.getInfo().ifMap.items():
    print(label, ifinfo.macAddr)

