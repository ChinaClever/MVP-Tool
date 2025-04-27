import serial
import time
import sys
import re
import subprocess

import threading
from raritan import rpc
from raritan.rpc import pdumodel
import raritan.rpc.peripheral
from raritan.rpc import usb
from raritan.rpc import usermgmt
from raritan.rpc import security

#import telnetlib
# 忽略弃用警告
import warnings
warnings.filterwarnings("ignore",category=DeprecationWarning)

# Replace with your actual COM port (e.g., COM1, COM2, etc.)


print(sys.argv[1])
print(sys.argv[2])
#COM_Num = input("Type in COM port number: ")
print("当前 COM 端口号： ")
COM_Num = sys.argv[1]
COM_PORT = "COM" + COM_Num  # Adjust based on your system
BAUD_RATE = 115200  # Adjust based on your device

def send_command(command):
    ser.write((command + '\n').encode())  # Send command (make sure to encode as bytes)
    time.sleep(1)  # Give the device time to process
    response = ser.readlines()  # Read the response (might need to adjust this depending on the device)
    response = [line.decode('utf-8').strip() for line in response]  # Decode bytes to string
    return response

# Open the serial connection

ser = serial.Serial(COM_PORT, baudrate=BAUD_RATE, timeout=2)
# Send 'root' command and press Enter to login
print("正在发送 'root' 进行登录...")
send_command("root")
# Send the 'root' command
print("已发送 'root'，现在正在等待 进入...")
send_command("")


# Send Enter (empty command simulates pressing Enter)

# Send Enter (empty command simulates pressing Enter)
Test_init = sys.argv[2]

# Function to send command and receive response

def verify_RTC_log(logs):
    # Define the regex pattern for matching the RTC log entries
    boot_reason_pattern = r"\[\s*\d+\.\d+\] Boot reason\(s\) 0x00000000: Power-on or pin-hole reset"
    rtc_vbackup_pattern = r"\[\s*\d+\.\d+\] rtc-rv3028 \d+-\d+: RTC was on Vbackup: Controller was powered off for a short time"
    rtc_registered_pattern = r"\[\s*\d+\.\d+\] rtc-rv3028 \d+-\d+: registered as rtc0"
    rtc_system_clock_pattern = r"\[\s*\d+\.\d+\] rtc-rv3028 \d+-\d+: setting system clock to \d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2} UTC \(\d+\)"

    # Check if all the expected patterns exist in the logs
    if re.search(boot_reason_pattern, logs) and \
       re.search(rtc_vbackup_pattern, logs) and \
       re.search(rtc_registered_pattern, logs) and \
       re.search(rtc_system_clock_pattern, logs):
        return True
    else:
        return False

log_pattern = r'\[(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2} [A-Za-z]+)\]:'  # Timestamp
log_pattern += r'\[(\w+)\]:'  # Log level (INF, ERR, etc.)
log_pattern += r'\[(\w+)\]:'  # First source component (e.g., cryptod)
log_pattern += r'\[(\w+)\]:'  # Second source component (e.g., cryptod)
log_pattern += r'\[([a-zA-Z0-9._]+:\d+)\]:'  # Filename and line number (e.g., AtcaDevice.cpp:55)
log_pattern += r'(.+)'  # The message content

# Compile the regex pattern
secure_log = re.compile(log_pattern)

# Function to check if a log entry matches the pattern
def verify_secure_log(log_line):
    match = secure_log.match(log_line)
    if match:
        return True
    else:
        print(f"日志格式无效： {log_line}")
        return False

def hex_to_celsius(hex_values):
    """Convert the hexadecimal values to Celsius temperature."""
    # Assuming 2-byte temperature data (e.g., 0x28 and 0xd0)
    # Combine the two hexadecimal bytes to form the full 16-bit value
    raw_data = (hex_values[0] << 8) + hex_values[1]

    # Assuming a temperature sensor that uses a specific formula
    # (this can vary based on the sensor, here we assume a basic conversion)
    celsius = raw_data / 16.0  # Example: divide by 16 (adjust if needed for specific sensor)

    return celsius

pattern = r"""
\[
    \d+\.\d+\]                # Match the timestamp, e.g. [1.894478]
    \s*Bluetooth: hci0: BCM:   # Match the fixed part: "Bluetooth: hci0: BCM:"
    (\w+: )?                   # Optional: Match words like "chip id", "features", etc.
    [^\n]+                      # Match the rest of the line (can vary, e.g., numbers, strings)
"""

# Function to check if the output matches the expected pattern
def check_bluetooth_output(result):
    # If result is a list, join it into a single string (join with '\n' between lines)
    if isinstance(result, list):
        result = "\n".join(result)

    # Split the input into lines after joining
    lines = result.strip().splitlines()

    # Compile the pattern with extended mode (allows multiline regex)
    compiled_pattern = re.compile(pattern, re.VERBOSE)

    # Check each line to see if it matches the pattern
    for line in lines:
        if not compiled_pattern.match(line.strip()):
            return True
    return False

def check_ip_addresses(response):
    # Define the interfaces to check for IP addresses
    interfaces = ['eth0', 'eth1', 'eth2']
    ip_addresses = {}

    for line in response:
        for interface in interfaces:
            if interface in line:  # Look for the interface line
                ip_match = re.search(r'inet (\d+\.\d+\.\d+\.\d+)', line)
                if ip_match:
                    ip_address = ip_match.group(1)
                    ip_addresses[interface] = ip_address

    # Check if all IP addresses for eth0, eth1, eth2 exist and start with 192.168
    for interface in interfaces:
        ip_address = ip_addresses.get(interface)
        if not ip_address or not ip_address.startswith("192.168"):
            return "FAILED"

    return "PASSED"

def extract_ip(response):
    ip_pattern = r"\d+\.\d+\.\d+\.\d+"  # Regular expression to match an IP address
    for line in response:
        match = re.search(ip_pattern, line)
        if match:
            return match.group(0)
    return None
ser.write(b'\x04')
time.sleep(2)
print("运行 MVP3 自动测试 ")
# Send command to get the IP address

response = send_command("")  # This can be the empty command or a specific one to get the IP

# Extract the IP address
device_ip = extract_ip(response)
if not device_ip:
    print("在响应中找不到 IP 地址。")
    sys.exit(1)  # Exit if no IP address is found

# Root command to be sent

root_command = 'root'
response = send_command(root_command)
enable_telnet = "cfgc set proto_listener[telnet_debug].enabled=1"
response = send_command(enable_telnet)
# --- Buzzer Test ---
if Test_init == "0" or Test_init == "1":
    print("正在运行蜂鸣器测试...")
    buzzer_command = "tfw_beep 100"
    attempts = 3
    while True:
        buzzer_response = send_command(buzzer_command)
        Buzzer_result = input("Did you hear the beep? Y/N: ").upper()

        if Buzzer_result == 'Y':  # If user inputs 'Y'
            print("1. 蜂鸣器 测试通过")
            break  # Exit the loop and proceed
        elif Buzzer_result == 'N':  # If user inputs 'N'
            print("未听到蜂鸣器，请重试")
            attempts -= 1  # Exit the script with error code 1
        else:
            print("输入无效。请输入 Y 或 N。")

        if attempts == 0:  # Check if attempts are exhausted
            print("1. 蜂鸣器 测试失败")
            sys.exit(1)
# --- RTC Test ---
if Test_init == "0" or Test_init == "2":
    print("正在运行 实时时钟 测试 ...")
    rtc_command = "dmesg | grep rtc"
    rtc_response = send_command(rtc_command)
    rtc_response = "\n".join(rtc_response)
    rtc_verify = verify_RTC_log(rtc_response)
    if rtc_response and rtc_verify:
        print("2. 实时时钟 测试通过")
    else:
        print("2. 实时时钟 测试失败")
        sys.exit(1)

# --- Debug-LED Test ---
if Test_init == "0" or Test_init == "3":
    print("正在 调试LED测试 ...")
    attempts = 3
    debug_led_command = "gpioapp set DEBUG_LED=1"

    while attempts > 0:
        debug_led_response = send_command(debug_led_command)
        debug_led_result = input("LED 是否闪烁？ Y/N: ").upper()

        if debug_led_result == 'Y':  # If user inputs 'Y'
            print("3. 调试LED测试 通过 ...")
            break  # Exit the loop and proceed
        elif debug_led_result == 'N':  # If user inputs 'N'
            print("未看到 调试LED测试，请重试")
            attempts -= 1  # Decrease the attempts
        else:
            print("输入无效. 请输入 Y or N.")

        if attempts == 0:  # Check if attempts are exhausted
            print("3. 调试LED 测试失败")
            sys.exit(1)

# --- Temperature Sensor Test ---
if Test_init == "0" or Test_init == "4":
    print("运行 PCB 温度传感器测试...")
    temperature_command = "i2capp -i 0x48 -c 2"
    temperature_response = send_command(temperature_command)
    #print(temperature_response)
    try:
        temp_data = temperature_response[2].split()[1:]
        # Debug print the split data
        decimal_numbers = [int(hex_num, 16) for hex_num in temp_data]
        decimal_figure = decimal_numbers[1] / 256
        result_temp = decimal_numbers[0] + decimal_figure
        result = str(result_temp) + "°C"
        print(result)
        print("4. PCB 温度传感器测试通过")
    # Print the results

    except Exception as e:
        print(f"读取温度传感器数据时出错： {e}")
        print("4. PCB 温度传感器测试失败")
        sys.exit(1)
#Error Message: Error reading temperature sensor data: invalid literal for int() with base 16: 'i2capp'

# Running Ethernet Test

# --- Secure Element Test ---
if Test_init == "0" or Test_init == "5":
    print("正在运行安全元件测试...")
    secure_element_command = "log_dump | grep cryptod"
    secure_element_response = send_command(secure_element_command)[1:-1]
    for line in secure_element_response:
        secure_element_verify = verify_secure_log(line)
        if secure_element_verify:
            continue
        else:
            print("5. 安全元件测试失败")
            sys.exit(1)
    print("5. 安全元件测试通过")

if Test_init == "0" or Test_init == "6":
    print("正在运行蓝牙测试...")
    Bluetooth__element_command = "dmesg | grep hci0"
    Bluetooth_response = send_command(Bluetooth__element_command)
    Bluetooth_result = check_bluetooth_output(Bluetooth_response)
    if Bluetooth_result:
        print("6. 蓝牙测试通过")
    else:
        print("6. 蓝牙测试失败")
        sys.exit(1)

if Test_init == "0" or Test_init == "7":
    print("正在运行以太网测试...")
    response = send_command("ip addr")

    # Check for 192.168.x.x IP addresses on eth0, eth1, and eth2
    result = check_ip_addresses(response)
    print(f"7. 以太网 测试 {result}")

if Test_init == "0" or Test_init == "8":
    print("正在运行 SPE接口 测试...")
    time.sleep(1)
    SPE_result = input("您是否看到两个 SPE 灯都变绿了？ (y/n):")
    if SPE_result.upper() == "Y":
        print("8. SPE 测试通过")
    elif SPE_result.upper() == "N":
        print(f"8. SPE 测试失败. 测试人员没有观察到绿色")
        sys.exit(1)
    else:
        print(f"以下项的输入无效： {description}. 请回答 'y' or 'n'.")

# Function to send command and receive response
def send_Zigbee_command(command):
    ser.write((command + '\n').encode())  # Send command (make sure to encode as bytes)
    time.sleep(1)  # Wait for response processing
    response = ser.readlines()  # Read the response
    response = [line.decode('utf-8').strip() for line in response]  # Decode bytes to string
    return response


def check_commissioning_state_on(response):
    # Check if "Commissioning state: on" is in the response
    if any("Commissioning state: on" in line for line in response):
        return True
    else:
        return False


def check_commissioning_state_off(response):
    # Check if "Commissioning state: off" is in the response
    if any("Commissioning state: off" in line for line in response):
        return True
    else:
        return False


def check_device_list_for_active(response):
    # Look for the pattern "Active" in the response for the device list
    active_found = False
    device_pattern = re.compile(r"\|[\s\S]+Active[\s\S]+\|")  # Regex for matching Active in table
    for line in response:
        if device_pattern.match(line):
            active_found = True
            break
    return active_found



def capture_device_output():
    """
    Capture any output from the peripheral device triggered by the pinhole button press.
    This function continuously reads from the serial port (or other interface)
    to catch any response generated by the device.
    """

    # Wait for some time to allow the device to generate output (adjust timing as necessary)
    time.sleep(10)  # Adjusted to wait for 10 seconds as in your request

    # Capture the output from the peripheral device.
    device_output = []
    while ser.in_waiting > 0:  # Check if there's any data in the input buffer
        line = ser.readline().decode('utf-8').strip()  # Read one line of output
        device_output.append(line)

    return device_output


def run_zigbee_test():
    print('正在运行 Zigbee 测试...')

    send_Zigbee_command('sgw_lapi_controller')
    send_Zigbee_command('channel_change 26 false')

    permit_join = send_Zigbee_command('permit_join true')
    if not check_commissioning_state_on(permit_join):
        return "9. ZIGBEE Test FAILED: Commissioning state did not turn on"

    print("按住 Zigbee 传感器上的针孔按钮一秒钟...")
    time.sleep(10)

    device_list = send_Zigbee_command('list_devices')
    if not check_commissioning_state_off(device_list):
        return "9. ZIGBEE Test FAILED: Commissioning state did not turn off"

    if not check_device_list_for_active(device_list):
        return "9. ZIGBEE Test FAILED: 'Active' not found in the device list"

    send_Zigbee_command('measurement_show_runtime true')

    print("请按下针孔按钮少于一秒钟...")

    # Capture the peripheral device output after the button press
    peripheral_device_output = capture_device_output()

    # Check if there was any output after the button press and the sleep period
    if len(peripheral_device_output) == 0:
        return "9. ZIGBEE Test FAILED: No output was generated by the peripheral device"
    else:
        return "9. ZIGBEE Test PASSED"

if Test_init == "0" or Test_init == "9":
    result = run_zigbee_test()
    print(result)
    ser.write(b'\x03')
# Close any open connections

commands = [
    ("Enable driver 1", "i2capp -i 0x32 -p 0x00 0x40"),
    ("Round RGB LEDs red", "i2capp -i 0x32 -p 0x16 0x80 0 0 0x80 0 0 0x80 0 0"),
    ("Round RGB LEDs green", "i2capp -i 0x32 -p 0x16 0 0x80 0 0 0x80 0 0 0x80 0"),
    ("Round RGB LEDs blue", "i2capp -i 0x32 -p 0x16 0 0 0x80 0 0 0x80 0 0 0x80"),
    ("Enable driver 2", "i2capp -i 0x33 -p 0x00 0x40"),
    ("Bottom RGB red", "i2capp -i 0x33 -p 0x16 0x80 0 0 0x80 0 0"),
    ("Bottom RGB green", "i2capp -i 0x33 -p 0x16 0 0x80 0 0 0x80 0"),
    ("Bottom RGB blue", "i2capp -i 0x33 -p 0x16 0 0 0x80 0 0 0x80")
]

if Test_init == "0" or Test_init == "10":
    print("正在运行 RGB灯 测试...")
    # Loop through all commands
    for description, command in commands:
        # Send the command
        send_command(command)

        # Skip asking for color change feedback for "Enable driver" commands
        if "Enable driver" in description:
            time.sleep(1)  # Wait for 2 seconds to let the system process the command
            continue

        # Wait for 2 seconds to allow the tester to observe the LED color change
        time.sleep(1)  # Pause for 2 seconds before asking

        # Ask the tester if they observed the color change
        feedback = input(f"您是否看到 的颜色变化: {description}? (y/n): ").strip().lower()

        # Record or print feedback (you can expand this to log the results if needed)
        if feedback == "y" and description == "Bottom RGB blue":
            print("10. RGB 灯 测试通过")
        elif feedback == "n":
            print(f"10. RGB 指示灯测试失败。测试人员未观察到 的颜色变化: {description}")
            break
        if feedback == "y":
            continue
        else:
            print(f"的输入无效: {description}. 请回答 'y' or 'n'.")

        # Optionally, you can insert a pause before proceeding to the next command
        time.sleep(1)


if Test_init == "0" or Test_init == "11":
    print("正在运行 RS485 和 Modbus 测试...")
    response = send_command('microcom /dev/ttyS3')
    serial_response_received = False

    def telnet_commands(device_ip):
        try:
            tn = telnetlib.Telnet(device_ip, port=24, timeout=30)

            tn.write(b"root\n")
            time.sleep(1)
            response1 = tn.read_very_eager().decode('utf-8')

            tn.write(b"microcom /dev/ttyS6\n")
            time.sleep(1)
            response2 = tn.read_very_eager().decode('utf-8')

            tn.write(b"Response from telnet connection\n")
            time.sleep(1)
            response3 = tn.read_very_eager().decode('utf-8')

            tn.write(b"exit\n")
            tn.close()

        except Exception as e:
            print(f"Telnet 错误: {e}")

    def serial_monitor():
        global serial_response_received
        while True:
            if ser.in_waiting:
                serial_response = ser.readline().decode('utf-8').strip()
                if serial_response:
                    if "Response from telnet connection" in serial_response:
                        serial_response_received = True
            time.sleep(0.1)

    def main():



        global serial_response_received
        serial_thread = threading.Thread(target=serial_monitor)
        serial_thread.daemon = True
        serial_thread.start()

        telnet_commands(device_ip)

        time.sleep(2) #give time for serial and telnet threads to finish.

        if serial_response_received:
            print("11. RS485 和 Modbus 测试通过")
        else:
            print("11. RS485 和 Modbus 测试失败")

    if __name__ == "__main__":
        main()

agent = rpc.Agent("https", device_ip, "admin", "Rar!tan0", disable_certificate_verification=True)

pdu = pdumodel.Pdu("/model/pdu/0", agent)
#裝置資訊
pdm = raritan.rpc.peripheral.DeviceManager("/model/peripheraldevicemanager", agent)

slots = pdm.getDeviceSlots()

sensor_data_found = False
usb_devices_count = 0

for num, slot in enumerate(slots):
    settings = slot.getSettings()
    device = slot.getDevice()
    if device is None:
        continue
    else:
        if device.device:
            if device.deviceID.type.readingtype == raritan.rpc.sensors.Sensor.NUMERIC:
                reading = device.device.getReading()
                sensor_data_found = True  # If sensor data is found, set the flag to True
            else:
                state = device.device.getState()
                print(" State: %d" % state.value)

if Test_init == "0" or Test_init == "12":
    if sensor_data_found:
        print("12. 传感器测试通过")
    else:
        print("12. 传感器测试失败")
ser.write(b'\0x18')
# USB Proxy to fetch USB devices information
usb_proxy = usb.Usb("/usb", agent)
usb_numbers = usb_proxy.getHostPortCount()
usb_info = usb_proxy.getDevices()

for device in usb_info:
    usb_devices_count += 1  # Increment the USB devices count

# Check if 2 USB devices and sensor data are found
if Test_init == "0" or Test_init == "13":
    if usb_devices_count == 2 and sensor_data_found:
        print("13. USB 测试通过")
    else:
        print("13. USB 测试失败")

# Exit the script
sys.stdout.write('Press Enter to exit.')
sys.stdin.readline()
sys.exit(0)



