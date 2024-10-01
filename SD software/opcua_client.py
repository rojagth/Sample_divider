import sys
sys.path.insert(0, "..")
from opcua import Client, ua
import time
import serial
import threading

# The URI we're interested in
namespace_uri = "http://examples.freeopcua.github.io"

# Handler for subscription
class SubHandler(object):
    def __init__(self, servoS_position, servoF_position, servoB_position, stepper_speed, stepper_state, ledY_state, ledG_state, ledR_state, sample_state):
        self.servoS_position = servoS_position
        self.servoF_position = servoF_position
        self.servoB_position = servoB_position
        self.stepper_speed = stepper_speed
        self.stepper_state = stepper_state
        self.ledY_state = ledY_state
        self.ledG_state = ledG_state
        self.ledR_state = ledR_state
        self.sample_state = sample_state

    # Data coming to Arduino
    def datachange_notification(self, node, val, data):
        print(f"New data change event -> Value: {val}")
        command = ""

        if node == self.servoS_position:
            command = f"SSERVO:{int(val)}\n"
        elif node == self.servoF_position:
            command = f"FSERVO:{int(val)}\n"
        elif node == self.servoB_position:
            command = f"BSERVO:{int(val)}\n"
        elif node == self.stepper_speed:
            command = f"STEPPERSPEED:{int(val)}\n"
        elif node == self.stepper_state:
            command = f"STEPPER:{int(val)}\n"
        elif node == self.ledY_state:
            command = f"LEDY:{int(val)}\n"
        elif node == self.ledG_state:
            command = f"LEDG:{int(val)}\n"
        elif node == self.ledR_state:
            command = f"LEDR:{int(val)}\n"
        elif node == self.sample_state:
            command = f"SAMPLE:{int(val)}\n"

        print(f"Sending command: {command.strip()}")
        ser.write(command.encode())

# Data coming from Arduino
def read_from_serial():
    while True:
        if ser.in_waiting > 0:
            serial_data = ser.readline().decode().strip()
            if serial_data:
                print(f"Received from Arduino: {serial_data}")

                if serial_data.startswith("SERVOS:"):
                    servoS_positionA = int(serial_data.split(':')[1])
                    servoS_position.set_value(servoS_positionA)

                elif serial_data.startswith("SERVOF:"):
                    servoF_positionA = int(serial_data.split(':')[1])
                    servoF_position.set_value(servoF_positionA)

                elif serial_data.startswith("SERVOB:"):
                    servoB_positionA = int(serial_data.split(':')[1])
                    servoB_position.set_value(servoB_positionA)

                elif serial_data.startswith("STEPPERSPEED:"):
                    speed_value = int(serial_data.split(':')[1])
                    stepper_speed.set_value(speed_value)

                elif "STEPPER:ON" in serial_data:
                    stepper_state.set_value(True)

                elif "STEPPER:OFF" in serial_data:
                    stepper_state.set_value(False)

                elif "LEDY:ON" in serial_data:
                    ledY_state.set_value(True)

                elif "LEDY:OFF" in serial_data:
                    ledY_state.set_value(False)

                elif "LEDG:ON" in serial_data:
                    ledG_state.set_value(True)

                elif "LEDG:OFF" in serial_data:
                    ledG_state.set_value(False)

                elif "LEDR:ON" in serial_data:
                    ledR_state.set_value(True)

                elif "LEDR:OFF" in serial_data:
                    ledR_state.set_value(False)

                elif "SAMPLE:ON" in serial_data:
                    sample_state.set_value(True)

                elif "SAMPLE:OFF" in serial_data:
                    sample_state.set_value(False)


if __name__ == "__main__":
    # Prompt user for input
    xinc = input("Enter increment length [m]: ")
    fraction_sample = input("Enter fraction to go into sample (if you want 1/10 please enter 0.1) [-]: ")

    client = Client("opc.tcp://192.168.1.100:4840/freeopcua/server/")
    ser = None
    subscription = None
    try:
        ser = serial.Serial('COM3', 57600, timeout=1)
        time.sleep(2)
        print("Serial connected")

        client.connect()
        print("Client connected")

        # Namespace index
        ns_array = client.get_namespace_array()
        ns_index = ns_array.index(namespace_uri)
        print(f"Namespace index is: {ns_index}")

        # Servo S
        servoS = client.get_node("ns=2;i=1001")
        servoS_position = client.get_node("ns=2;i=1002")
        servoS_position_value = servoS_position.get_value()

        # Servo F
        servoF = client.get_node("ns=2;i=1003")
        servoF_position = client.get_node("ns=2;i=1004")
        servoF_position_value = servoF_position.get_value()

        # Servo B
        servoB = client.get_node("ns=2;i=1005")
        servoB_position = client.get_node("ns=2;i=1006")
        servoB_position_value = servoB_position.get_value()

        # Stepper
        stepper = client.get_node("ns=2;i=1007")
        stepper_speed = client.get_node("ns=2;i=1008")
        stepper_speed_value = stepper_speed.get_value()
        stepper_realspeed = client.get_node("ns=2;i=1009")
        stepper_realspeed_value = stepper_realspeed.get_value()
        stepper_state = client.get_node("ns=2;i=1010")
        stepper_state_value = stepper_state.get_value()

        # LED Y
        ledY = client.get_node("ns=2;i=1011")
        ledY_state = client.get_node("ns=2;i=1012")
        ledY_state_val = ledY_state.get_value()

        # LED G
        ledG = client.get_node("ns=2;i=1013")
        ledG_state = client.get_node("ns=2;i=1014")
        ledG_state_val = ledG_state.get_value()

        # LED R
        ledR = client.get_node("ns=2;i=1015")
        ledR_state = client.get_node("ns=2;i=1016")
        ledR_state_val = ledR_state.get_value()

        # SAMPLE
        sample = client.get_node("ns=2;i=1017")
        sample_state = client.get_node("ns=2;i=1018")
        sample_state_val = ledR_state.get_value()

        # Initialize SubHandler with correct node objects
        handler = SubHandler(servoS_position, servoF_position, servoB_position, stepper_speed, stepper_state, ledY_state, ledG_state, ledR_state, sample_state)

        # Create a subscription and subscribe to changes
        subscription = client.create_subscription(1000, handler)
        handle1 = subscription.subscribe_data_change(servoS_position)
        handle2 = subscription.subscribe_data_change(servoF_position)
        handle3 = subscription.subscribe_data_change(servoB_position)
        handle4 = subscription.subscribe_data_change(stepper_speed)
        handle5 = subscription.subscribe_data_change(stepper_state)
        handle6 = subscription.subscribe_data_change(ledY_state)
        handle7 = subscription.subscribe_data_change(ledG_state)
        handle8 = subscription.subscribe_data_change(ledR_state)
        handle9 = subscription.subscribe_data_change(sample_state)

        print("Commands: \n - sServo <0-180> \n - fServo <0-180> \n - bServo <0-180> \n - Stepperspeed <700-1200> \n - Stepper on/off \n - LedY on/off \n - LedG on/off \n - LedR on/off \n - Sample on/off \n - exit")

        # Send values to Arduino
        ser.write(f"xinc:{xinc}\n".encode())
        ser.write(f"fraction_sample:{fraction_sample}\n".encode())
        print("Values sent to Arduino.")

        serial_thread = threading.Thread(target=read_from_serial, daemon=True)
        serial_thread.start()

        while True:
            cmd = input("Enter command: ").strip().lower()

            if cmd == "exit":
                break
            
            # Communication with the server
            elif cmd.startswith("sservo "):
                _, servoS_position_str = cmd.split()
                try:
                    servoS_position_int = int(servoS_position_str)
                    servoS_position.set_value(ua.Variant(servoS_position_int, ua.VariantType.Int16))
                except ValueError:
                    print("Invalid position of servo S. Please enter an integer between 0-180.")

            elif cmd.startswith("fservo "):
                _, servoF_position_str = cmd.split()
                try:
                    servoF_position_int = int(servoF_position_str)
                    servoF_position.set_value(ua.Variant(servoF_position_int, ua.VariantType.Int16))
                except ValueError:
                    print("Invalid position of servo F. Please enter an integer between 0-180.")

            elif cmd.startswith("bservo "):
                _, servoB_position_str = cmd.split()
                try:
                    servoB_position_int = int(servoB_position_str)
                    servoB_position.set_value(ua.Variant(servoB_position_int, ua.VariantType.Int16))
                except ValueError:
                    print("Invalid position of servo B. Please enter an integer between 0-180.")

            elif cmd.startswith("stepperspeed "):
                _, stepper_speed_str = cmd.split()
                try:
                    speed = int(stepper_speed_str)
                    stepper_speed.set_value(ua.Variant(speed, ua.VariantType.Int16))
                except ValueError:
                    print("Invalid speed. Please enter an integer between 0-1000.")

            elif cmd.startswith("stepper "):
                _, stepper_state_str = cmd.split()
                stepper_state_t = True if stepper_state_str == "on" else False
                stepper_state.set_value(ua.Variant(stepper_state_t, ua.VariantType.Boolean))

            elif cmd.startswith("yled "):
                _, ledY_state_str = cmd.split()
                ledY_state_t = True if ledY_state_str == "on" else False
                ledY_state.set_value(ua.Variant(ledY_state_t, ua.VariantType.Boolean))

            elif cmd.startswith("gled "):
                _, ledG_state_str = cmd.split()
                ledG_state_t = True if ledG_state_str == "on" else False
                ledG_state.set_value(ua.Variant(ledG_state_t, ua.VariantType.Boolean))

            elif cmd.startswith("rled "):
                _, ledR_state_str = cmd.split()
                ledR_state_t = True if ledR_state_str == "on" else False
                ledR_state.set_value(ua.Variant(ledR_state_t, ua.VariantType.Boolean))

            elif cmd.startswith("sample "):
                _, sample_state_str = cmd.split()
                sample_state_t = True if sample_state_str == "on" else False
                sample_state.set_value(ua.Variant(sample_state_t, ua.VariantType.Boolean))

            else:
                print("Unknown command.")

            time.sleep(0.1)
    except Exception as e:
        ledR_state.set_value(ua.Variant(True, ua.VariantType.Boolean))

    finally:
        if subscription is not None:
            subscription.delete()
        if client:
            client.disconnect()
        if ser:
            ser.close()