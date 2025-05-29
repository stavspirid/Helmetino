import tkinter as tk
from PIL import Image, ImageTk
import threading
import time
import serial


temperature_values = []
led = False
sound = False
alert = False
start_time = time.time()

# Arduino
arduino = serial.Serial(port='COM12', baudrate=9600, timeout=.1)
ser = arduino

# Windows
window = tk.Tk()
window.title("Helmetino")
window.geometry("600x400")
window.configure(bg="#1e1e1e")

# Image initialization
led_on_img = ImageTk.PhotoImage(Image.open("./images/led.png").resize((60, 60)))
led_off_img = ImageTk.PhotoImage(Image.open("./images/led_off.png").resize((60, 60)))
speaker_on_img = ImageTk.PhotoImage(Image.open("./images/sound.png").resize((60, 60)))
speaker_off_img = ImageTk.PhotoImage(Image.open("./images/sound_mute.png").resize((60, 60)))
alarm_img = ImageTk.PhotoImage(Image.open("./images/alarm.png").resize((60, 60)))
cyclist_img = ImageTk.PhotoImage(Image.open("./images/bike.png").resize((60, 60)))

# Helmetino
title1 = tk.Label(window, text="Helmetino", font=("Helvetica", 28, "bold"), bg="#1e1e1e", fg="white")
title2 = tk.Label(window, text="Rider ID: 7008797\n", font=("Helvetica", 15, "bold"), bg="#1e1e1e", fg="white")
title1.pack(pady=5)
title2.pack(pady=10)

# Container
main_frame = tk.Frame(window, bg="#1e1e1e")
main_frame.pack(fill="both", expand=True)

#  Temperature box
temp_frame = tk.Frame(main_frame, bg="#A8E6CF", bd=2, relief="groove", padx=10, pady=10)
temp_frame.grid(row=0, column=0, padx=15, pady=10)

temp_value_label = tk.Label(temp_frame, text="-- °C", font=("Helvetica", 24), bg="#A8E6CF", fg="white")
temp_avg_label = tk.Label(temp_frame, text="Μ.Ο.: -- °C", font=("Helvetica", 12), bg="#A8E6CF", fg="gray")

temp_value_label.pack()
temp_avg_label.pack()

# Led box
led_frame = tk.Frame(main_frame, bg="#B3E5FC", bd=2, relief="groove", width=150, height=150)
led_frame.grid(row=0, column=1, padx=15, pady=10)
led_frame.grid_propagate(False)

led_icon = tk.Label(led_frame, bg="#B3E5FC")
led_icon.place(relx=0.5, rely=0.3, anchor="center")

led_status_label = tk.Label(led_frame, text="Led is OFF", font=("Helvetica", 12), bg="#B3E5FC", fg="gray")
led_status_label.place(relx=0.5, rely=0.8, anchor="center")
led_frame.grid_propagate(False)


# Sound box / alert
speaker_frame = tk.Frame(main_frame, bg="#FF8A80", bd=2, relief="groove", width=150, height=150)
speaker_frame.grid(row=0, column=2, padx=15, pady=10)
speaker_frame.grid_propagate(False)

speaker_icon = tk.Label(speaker_frame, image=speaker_off_img, bg="#FF8A80")
speaker_icon.place(relx=0.5, rely=0.3, anchor="center")

speaker_status_label = tk.Label(speaker_frame, text="Sound is OFF", font=("Helvetica", 12), bg="#FF8A80", fg="gray")
speaker_status_label.place(relx=0.5, rely=0.8, anchor="center")
speaker_frame.grid_propagate(False)

speaker_icon.pack()
speaker_status_label.pack()

# Fallen
fallen_frame = tk.Frame(main_frame, bd=2, relief="groove", width=150, height=150)
fallen_frame.grid(row=0, column=3, padx=15, pady=10)
fallen_frame.grid_propagate(False)

cyclist_icon = tk.Label(fallen_frame, image=cyclist_img)
cyclist_icon.place(relx=0.5, rely=0.3, anchor="center")

fallen_status_label = tk.Label(fallen_frame, text="The cyclist is on track", font=("Helvetica", 12), fg="gray")
fallen_status_label.place(relx=0.5, rely=0.5, anchor="center")
fallen_frame.grid_propagate(False)

cyclist_icon.pack()
fallen_status_label.pack()
fallen_state = False

def read_from_arduino():
    global led, sound, alert, temperature_values
    data = {}  # Buffer για την αποθήκευση τιμών
    required_keys = {"TEMP", "LIGHT", "SOUND", "CRASH"}

    while True:
        try:
            line = ser.readline().decode('utf-8').strip()
            if line:
                print(f"Raw line: {line}")  # Debug

                if ':' in line:
                    key, val = line.split(':', 1)
                    data[key] = val

                if required_keys.issubset(data.keys()):
                    try:
                        temp = float(data.get('TEMP', 0))
                    except ValueError:
                        temp = 0.0

                    led_status = data.get('LIGHT') == '1'
                    sound_status = data.get('SOUND') == '1'
                    fallen_status = data.get('CRASH') == '1'

                    temperature_values.append(temp)

                    window.after(0, update_ui, temp, led_status, sound_status, fallen_status)

                    global start_time
                    if time.time() - start_time >= 10:
                        avg = sum(temperature_values) / len(temperature_values)
                        window.after(0, update_avg_temp, avg)
                        temperature_values.clear()
                        start_time = time.time()

                    # Reset data buffer
                    data.clear()

        except Exception as e:
            print("Parsing error:", e)

def update_ui(temp, led_status, sound_status, fallen_status):
    print(f"Temp: {temp:.1f} °C, LED: {led_status}, Sound: {sound_status}, Fallen: {fallen_status}")
    temp_value_label.config(text=f"{temp:.1f} °C")

    led_icon.config(image=led_on_img if led_status else led_off_img)
    led_status_label.config(
        text="Led is ON" if led_status else "Led is OFF",
        fg="lime" if led_status else "gray"
    )


    speaker_icon.config(image=speaker_on_img if sound_status else speaker_off_img)
    speaker_status_label.config(
        text="Sound is ON" if sound_status else "Sound is OFF",
        fg="lime" if sound_status else "gray"
    )

    # Update fallen label
    if fallen_status:
        cyclist_icon.config(image=alarm_img)
        fallen_status_label.config(text="The cyclist has crushed", fg="red")
    else:
        fallen_status_label.config(text="The cyclist is on track", fg="gray")

def update_avg_temp(avg):
    temp_avg_label.config(text=f"Μ.Ο.: {avg:.1f} °C")


threading.Thread(target=read_from_arduino, daemon=True).start()
window.mainloop()
