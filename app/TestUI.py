import tkinter as tk
from tkinter import ttk, messagebox
import serial
import json

# Function to send command and monitor response
def send_command(command):
    try:
        # Open the serial port with the selected parameters
        ser = serial.Serial(port=comport.get(), baudrate=int(baudrate.get()), timeout=1)

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
        
        # Close the serial connection
        ser.close()

        # Notify user that the command was sent
        messagebox.showinfo("Command Sent", "Command sent successfully!")

    except Exception as e:
        messagebox.showerror("Error", f"Failed to send command: {e}")

# Function to create and send start system command
def send_start_system():
    command = json.dumps({"command": "STARTSYSTEM"})
    send_command(command)

# Function to create and send stop system command
def send_stop_system():
    command = json.dumps({"command": "STOPSYSTEM"})
    send_command(command)

# Function to create and send motor command
def send_motor_command():
    motor_type = motor_type_combobox.get()
    speed = motor_speed_slider.get()  # Get value from the slider
    microsteps = motor_microsteps_combobox.get()
    direction = motor_direction_combobox.get()
    
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
        "stop": int(stop),
        "stepstotake": int(steps_to_take)
    })
    
    send_command(command)

# Set up the main window
root = tk.Tk()
root.title("Serial Command Sender")

# Com Port and Baud Rate settings
ttk.Label(root, text="Select COM Port:").grid(row=0, column=0, padx=10, pady=5)
comport = ttk.Combobox(root, values=["COM1", "COM2", "COM3", "COM4", "COM5"])  # Modify as needed for your system
comport.set("COM1")
comport.grid(row=0, column=1, padx=10, pady=5)

ttk.Label(root, text="Select Baud Rate:").grid(row=1, column=0, padx=10, pady=5)
baudrate = ttk.Combobox(root, values=[9600, 115200, 19200, 38400])  # Add more baud rates if needed
baudrate.set(115200)
baudrate.grid(row=1, column=1, padx=10, pady=5)

# Send "Start System" command button
start_button = ttk.Button(root, text="Start System", command=send_start_system)
start_button.grid(row=2, column=0, pady=10)

# Send "Stop System" command button
stop_button = ttk.Button(root, text="Stop System", command=send_stop_system)
stop_button.grid(row=2, column=1, pady=10)

# Motor Control Section
ttk.Label(root, text="Motor Type:").grid(row=3, column=0, padx=10, pady=5)
motor_type_combobox = ttk.Combobox(root, values=["motorCase", "motorDisc"])
motor_type_combobox.set("motorCase")
motor_type_combobox.grid(row=3, column=1, padx=10, pady=5)

ttk.Label(root, text="Speed:").grid(row=4, column=0, padx=10, pady=5)
motor_speed_slider = tk.Scale(root, from_=0, to=100, orient="horizontal")  # Slider for speed range
motor_speed_slider.set(50)  # Set default value
motor_speed_slider.grid(row=4, column=1, padx=10, pady=5)

ttk.Label(root, text="Microsteps:").grid(row=5, column=0, padx=10, pady=5)
motor_microsteps_combobox = ttk.Combobox(root, values=[1, 2, 4, 8, 16])
motor_microsteps_combobox.set(16)
motor_microsteps_combobox.grid(row=5, column=1, padx=10, pady=5)

ttk.Label(root, text="Direction:").grid(row=6, column=0, padx=10, pady=5)
motor_direction_combobox = ttk.Combobox(root, values=[0, 1])  # 0 for one direction, 1 for the opposite direction
motor_direction_combobox.set(0)
motor_direction_combobox.grid(row=6, column=1, padx=10, pady=5)

# Send Motor command button
motor_button = ttk.Button(root, text="Send Motor Command", command=send_motor_command)
motor_button.grid(row=7, column=0, columnspan=2, pady=10)

# Sensor Control Section
ttk.Label(root, text="Stop:").grid(row=8, column=0, padx=10, pady=5)
sensor_stop_entry = ttk.Entry(root)
sensor_stop_entry.insert(0, "1000")
sensor_stop_entry.grid(row=8, column=1, padx=10, pady=5)

ttk.Label(root, text="Steps to Take:").grid(row=9, column=0, padx=10, pady=5)
sensor_steps_entry = ttk.Entry(root)
sensor_steps_entry.insert(0, "200")
sensor_steps_entry.grid(row=9, column=1, padx=10, pady=5)

# Send Sensor command button
sensor_button = ttk.Button(root, text="Send Sensor Command", command=send_sensor_command)
sensor_button.grid(row=10, column=0, columnspan=2, pady=10)

# Log Section to display sent/received commands
log_text = tk.Text(root, width=60, height=15, wrap=tk.WORD)
log_text.grid(row=11, column=0, columnspan=2, padx=10, pady=10)
log_text.insert(tk.END, "Command Log:\n\n")

# Run the GUI
root.mainloop()
