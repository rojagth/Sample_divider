import sys
sys.path.insert(0, "..")
import time
from opcua import ua, Server

if __name__ == "__main__":

    server = Server()
    server.set_endpoint("opc.tcp://192.168.1.100:4840/freeopcua/server/")
    uri = "http://examples.freeopcua.github.io"
    idx = server.register_namespace(uri)

    # get Objects node
    objects = server.get_objects_node()

    # Servo S
    servoS_id = ua.NodeId(1001, idx)
    servoS = objects.add_object(servoS_id, "Servo S")

    servoS_position_id = ua.NodeId(1002, idx)
    servoS_position = servoS.add_variable(servoS_position_id, "Servo S position", 110.0)
    servoS_position.set_writable()

    # Servo F
    servoF_id = ua.NodeId(1003, idx)
    servoF = objects.add_object(servoF_id, "Servo F")

    servoF_position_id = ua.NodeId(1004, idx)
    servoF_position = servoF.add_variable(servoF_position_id, "Servo F position", 150.0)
    servoF_position.set_writable()

    # Servo B
    servoB_id = ua.NodeId(1005, idx)
    servoB = objects.add_object(servoB_id, "Servo B")

    servoB_position_id = ua.NodeId(1006, idx)
    servoB_position = servoB.add_variable(servoB_position_id, "Servo B position", 60.0)
    servoB_position.set_writable()

    # Stepper
    stepper_id = ua.NodeId(1007, idx)
    stepper = objects.add_object(stepper_id, "Stepper")

    stepper_speed_id = ua.NodeId(1008, idx)
    stepper_speed = stepper.add_variable(stepper_speed_id, "Stepper speed", 1000.0)
    stepper_speed.set_writable()

    stepper_realspeed_id = ua.NodeId(1009, idx)
    stepper_realspeed = stepper.add_variable(stepper_realspeed_id, "Stepper real speed", 0.0)
    stepper_realspeed.set_writable()

    stepper_state_id = ua.NodeId(1010, idx)
    stepper_state = stepper.add_variable(stepper_state_id, "Stepper state", False)
    stepper_state.set_writable()

    # LED Y
    ledY_id = ua.NodeId(1011, idx)
    ledY = objects.add_object(ledY_id, "Led Y")

    ledY_state_id = ua.NodeId(1012, idx)
    ledY_state = ledY.add_variable(ledY_state_id, "Led Y state", False)
    ledY_state.set_writable()

    # LED G
    ledG_id = ua.NodeId(1013, idx)
    ledG = objects.add_object(ledG_id, "Led G")

    ledG_state_id = ua.NodeId(1014, idx)
    ledG_state = ledG.add_variable(ledG_state_id, "Led G state", False)
    ledG_state.set_writable()

    # LED R
    ledR_id = ua.NodeId(1015, idx)
    ledR = objects.add_object(ledR_id, "Led R")

    ledR_state_id = ua.NodeId(1016, idx)
    ledR_state = ledR.add_variable(ledR_state_id, "Led R state", False)
    ledR_state.set_writable()

    # SAMPLE
    sample_id = ua.NodeId(1017, idx)
    sample = objects.add_object(sample_id, "Sample")

    sample_state_id = ua.NodeId(1018, idx)
    sample_state = sample.add_variable(sample_state_id, "Sample state", False)
    sample_state.set_writable()

    server.start()
    print("Server started at {}".format(server.endpoint))
    
    try:
        while True:
            time.sleep(1)
    finally:
        server.stop()