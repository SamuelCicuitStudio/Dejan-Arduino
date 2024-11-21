import tkinter as tk
from tkinter import ttk, messagebox
import serial
import json

# Global variable for serial connection
ser = None

# Function to send command and monitor response
def send_command(command):
    global ser
    if ser is None or not ser.is_open:
        messagebox.showerror("Error", "Serial port is not connected.")
        return

    try:
        # Send the command as a JSON object
        ser.write(command.encode('utf-8'))
        ser.flush()

        # Log sent command in the UI
        log_text.insert(tk.END, f"Sent Command:\n{command}\n\n")
        
        # Read response from ESP32
        response = ser.readline().decode('utf-8').strip()  # Read response line
        if response:
            print(f"Response from ESP32: {response}")
            log_text.insert(tk.END, f"Received Response:\n{response}\n\n")
        else:
            print("No response from ESP32.")
            log_text.insert(tk.END, f"Received Response:\nNo response from ESP32.\n\n")
        
        # Add separator line after each command-response pair
        log_text.insert(tk.END, "=========================================================================\n\n")
        
        # Notify user that the command was sent
        messagebox.showinfo("Command Sent", "Command sent successfully!")

    except Exception as e:
        messagebox.showerror("Error", f"Failed to send command: {e}")

# Function to request the current state from the ESP32 and update the UI
def request_current_state():
    global ser
    if ser is None or not ser.is_open:
        messagebox.showerror("Error", "Serial port is not connected.")
        return

    try:
        # Send command to request current state as JSON
        command = json.dumps({"command": "GETSTATUS"})
        send_command(command)

        # Wait for the response (ensure the response is complete)
        response = ser.readline().decode('utf-8').strip()
        if response:
            print(f"Response from ESP32: {response}")
            try:
                state_data = json.loads(response)

                # Parse and update the UI fields based on the received JSON
                if state_data.get("status") == "ok":
                    update_motor_case(state_data.get("motorCase", {}))
                    update_motor_disc(state_data.get("motorDisc", {}))
                    update_sensor(state_data.get("sensor", {}))
                    
                    # Update System Status (if needed)
                    system_data = state_data.get("system", {})
                    print(f"System status: {system_data.get('status', 'unknown')}")
                    print(f"Last command executed: {system_data.get('lastCommand', 'none')}")
                    
                    # Notify the user that the state has been updated
                    messagebox.showinfo("State Update", "The device state has been successfully updated.")
                else:
                    messagebox.showerror("Error", "Failed to retrieve device state.")
            except json.JSONDecodeError:
                messagebox.showerror("Error", "Failed to parse response from ESP32.")
        else:
            messagebox.showerror("Error", "No response from ESP32.")
    except Exception as e:
        messagebox.showerror("Error", f"Failed to request current state: {e}")

# Update motor control values for case motor
def update_motor_case(motor_case_data):
    motor_speed_slider_case.set(motor_case_data.get("speed", 50))
    motor_microsteps_combobox_case.set(motor_case_data.get("microsteps", 16))
    motor_direction_combobox_case.set(motor_case_data.get("direction", 0))

# Update motor control values for disc motor
def update_motor_disc(motor_disc_data):
    motor_speed_slider_disc.set(motor_disc_data.get("speed", 50))
    motor_microsteps_combobox_disc.set(motor_disc_data.get("microsteps", 16))
    motor_direction_combobox_disc.set(motor_disc_data.get("direction", 0))

# Update sensor control values
def update_sensor(sensor_data):
    sensor_stop_entry.delete(0, tk.END)
    sensor_stop_entry.insert(0, sensor_data.get("stop", 1000))
    sensor_steps_entry.delete(0, tk.END)
    sensor_steps_entry.insert(0, sensor_data.get("stepsToTake", 200))

# Function to create and send start system command
def send_start_system():
    command = json.dumps({"command": "STARTSYSTEM"})
    send_command(command)

# Function to create and send stop system command
def send_stop_system():
    command = json.dumps({"command": "STOPSYSTEM"})
    send_command(command)

# Motor control for Case Motor
def send_case_motor_command():
    motor_type = "motorCase"
    speed = motor_speed_slider_case.get()
    microsteps = motor_microsteps_combobox_case.get()
    direction = motor_direction_combobox_case.get()
    
    command = json.dumps({
        "command": "motor",
        "motorType": motor_type,
        "speed": float(speed),
        "microsteps": int(microsteps),
        "direction": int(direction)
    })
    
    send_command(command)

# Motor control for Disc Motor
def send_disc_motor_command():
    motor_type = "motorDisc"
    speed = motor_speed_slider_disc.get()
    microsteps = motor_microsteps_combobox_disc.get()
    direction = motor_direction_combobox_disc.get()
    
    command = json.dumps({
        "command": "motor",
        "motorType": motor_type,
        "speed": float(speed),
        "microsteps": int(microsteps),
        "direction": int(direction)
    })
    
    send_command(command)

# Function to create and send sensor command
def send_sensor_command():
    stop = sensor_stop_entry.get()
    steps_to_take = sensor_steps_entry.get()
    
    command = json.dumps({
        "command": "sensor",
        "stoptime": int(stop),
        "stepstotake": int(steps_to_take)
    })
    
    send_command(command)

# Function to connect to serial port and request current state
def connect_serial():
    global ser
    try:
        ser = serial.Serial(port=comport.get(), baudrate=int(baudrate.get()), timeout=1)
        # Change connect button to green if successful
        messagebox.showinfo("Connection Successful", "Serial connection established successfully.")
        
        # Request current state after successful connection
        
        
    except Exception as e:
        messagebox.showerror("Error", f"Failed to connect to serial port: {e}")

# Set up the main window
root = tk.Tk()
root.title("Serial Command Sender")

# Com Port and Baud Rate settings
ttk.Label(root, text="Select COM Port:").grid(row=0, column=0, padx=10, pady=5)
comport = ttk.Combobox(root, values=["COM1", "COM2", "COM3", "COM4", "COM5"])  # Modify as needed for your system
comport.set("COM1")
comport.grid(row=0, column=1, padx=10, pady=5)

# Request State Button
request_state_button = ttk.Button(root, text="Request State", command=request_current_state)
request_state_button.grid(row=2, column=1, pady=10)

ttk.Label(root, text="Select Baud Rate:").grid(row=1, column=0, padx=10, pady=5)
baudrate = ttk.Combobox(root, values=[9600, 115200, 19200, 38400])  # Add more baud rates if needed
baudrate.set(115200)
baudrate.grid(row=1, column=1, padx=10, pady=5)

# Connect Button
connect_button = ttk.Button(root, text="Connect", command=connect_serial)
connect_button.grid(row=2, column=0, columnspan=2, pady=10)

# Start and Stop System buttons
start_button = ttk.Button(root, text="Start System", command=send_start_system)
start_button.grid(row=3, column=0, pady=10)

stop_button = ttk.Button(root, text="Stop System", command=send_stop_system)
stop_button.grid(row=3, column=1, pady=10)

# Motor Control Section for Case Motor
ttk.Label(root, text="Case Motor Speed:").grid(row=4, column=0, padx=10, pady=5)
motor_speed_slider_case = tk.Scale(root, from_=1, to=460, orient="horizontal")
motor_speed_slider_case.set(250)
motor_speed_slider_case.grid(row=4, column=1, padx=10, pady=5)

ttk.Label(root, text="Microsteps (Case):").grid(row=5, column=0, padx=10, pady=5)
motor_microsteps_combobox_case = ttk.Combobox(root, values=[1, 2, 4, 8, 16])
motor_microsteps_combobox_case.set(1)
motor_microsteps_combobox_case.grid(row=5, column=1, padx=10, pady=5)

ttk.Label(root, text="Direction (Case):").grid(row=6, column=0, padx=10, pady=5)
motor_direction_combobox_case = ttk.Combobox(root, values=[0, 1])
motor_direction_combobox_case.set(0)
motor_direction_combobox_case.grid(row=6, column=1, padx=10, pady=5)

# Send Case Motor Command Button
send_case_motor_button = ttk.Button(root, text="Send Case Motor Command", command=send_case_motor_command)
send_case_motor_button.grid(row=7, column=0, columnspan=2, pady=10)

# Motor Control Section for Disc Motor
ttk.Label(root, text="Disc Motor Speed:").grid(row=8, column=0, padx=10, pady=5)
motor_speed_slider_disc = tk.Scale(root, from_=1, to=160, orient="horizontal")
motor_speed_slider_disc.set(120)
motor_speed_slider_disc.grid(row=8, column=1, padx=10, pady=5)

ttk.Label(root, text="Microsteps (Disc):").grid(row=9, column=0, padx=10, pady=5)
motor_microsteps_combobox_disc = ttk.Combobox(root, values=[1, 2, 4, 8, 16])
motor_microsteps_combobox_disc.set(1)
motor_microsteps_combobox_disc.grid(row=9, column=1, padx=10, pady=5)

ttk.Label(root, text="Direction (Disc):").grid(row=10, column=0, padx=10, pady=5)
motor_direction_combobox_disc = ttk.Combobox(root, values=[0, 1])
motor_direction_combobox_disc.set(0)
motor_direction_combobox_disc.grid(row=10, column=1, padx=10, pady=5)

# Send Disc Motor Command Button
send_disc_motor_button = ttk.Button(root, text="Send Disc Motor Command", command=send_disc_motor_command)
send_disc_motor_button.grid(row=11, column=0, columnspan=2, pady=10)

# Sensor Control Section
ttk.Label(root, text="Sensor Stop:").grid(row=12, column=0, padx=10, pady=5)
sensor_stop_entry = ttk.Entry(root)
sensor_stop_entry.grid(row=12, column=1, padx=10, pady=5)

ttk.Label(root, text="Sensor Steps to Take:").grid(row=13, column=0, padx=10, pady=5)
sensor_steps_entry = ttk.Entry(root)
sensor_steps_entry.grid(row=13, column=1, padx=10, pady=5)

# Send Sensor Command Button
send_sensor_button = ttk.Button(root, text="Send Sensor Command", command=send_sensor_command)
send_sensor_button.grid(row=14, column=0, columnspan=2, pady=10)

# Logs Section
log_text = tk.Text(root, height=10, width=60)
log_text.grid(row=15, column=0, columnspan=2, padx=10, pady=10)

# Start the Tkinter event loop
root.mainloop()
