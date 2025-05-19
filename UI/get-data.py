import tkinter as tk
from PIL import Image, ImageTk
import threading
import time
import serial


temperature_values = []
microphone = False
sound = False
alert = False
start_time = time.time()

# Arduino
arduino = serial.Serial(port='COM4', baudrate=115200, timeout=.1)
ser = arduino

# Windows
window = tk.Tk()
window.title("Helmetino")
window.geometry("600x400")
window.configure(bg="#1e1e1e")

# Image initialization
mic_on_img = ImageTk.PhotoImage(Image.open("./images/microphone.png").resize((60, 60)))
mic_off_img = ImageTk.PhotoImage(Image.open("./images/microphone_mute.png").resize((60, 60)))
speaker_on_img = ImageTk.PhotoImage(Image.open("./images/sound.png").resize((60, 60)))
speaker_off_img = ImageTk.PhotoImage(Image.open("./images/sound_mute.png").resize((60, 60)))
alarm_img = ImageTk.PhotoImage(Image.open("./images/alarm.png").resize((60, 60)))

# Helmetino
title = tk.Label(window, text="Helmetino", font=("Palatino Linotype", 28, "bold"), bg="#1e1e1e", fg="white")
title.pack(pady=10)

# Container
main_frame = tk.Frame(window, bg="#1e1e1e")
main_frame.pack(fill="both", expand=True)

#  Temperature box
temp_frame = tk.Frame(main_frame, bg="#A8E6CF", bd=2, relief="groove", padx=10, pady=10)
temp_frame.grid(row=0, column=0, padx=15, pady=10)

temp_value_label = tk.Label(temp_frame, text="-- °C", font=("Palatino Linotype", 24), bg="#A8E6CF", fg="white")
temp_avg_label = tk.Label(temp_frame, text="Μ.Ο.: -- °C", font=("Palatino Linotype", 12), bg="#A8E6CF", fg="gray")

temp_value_label.pack()
temp_avg_label.pack()

# Microphone box
mic_frame = tk.Frame(main_frame, bg="#B3E5FC", bd=2, relief="groove", width=150, height=150)
mic_frame.grid(row=0, column=1, padx=15, pady=10)
mic_frame.grid_propagate(False)

mic_icon = tk.Label(mic_frame, bg="#B3E5FC")
mic_icon.place(relx=0.5, rely=0.3, anchor="center")

mic_status_label = tk.Label(mic_frame, text="ΑΝΕΝΕΡΓΟ", font=("Palatino Linotype", 12), bg="#B3E5FC", fg="gray")
mic_status_label.place(relx=0.5, rely=0.8, anchor="center")
mic_frame.grid_propagate(False)


# Sound box / alert
speaker_frame = tk.Frame(main_frame, bg="#FF8A80", bd=2, relief="groove", width=150, height=150)
speaker_frame.grid(row=0, column=2, padx=15, pady=10)
speaker_frame.grid_propagate(False)

speaker_icon = tk.Label(speaker_frame, image=speaker_off_img, bg="#FF8A80")
speaker_icon.place(relx=0.5, rely=0.3, anchor="center")

speaker_status_label = tk.Label(speaker_frame, text="ΑΝΕΝΕΡΓΟ", font=("Palatino Linotype", 12), bg="#FF8A80", fg="gray")
speaker_status_label.place(relx=0.5, rely=0.8, anchor="center")
speaker_frame.grid_propagate(False)

speaker_icon.pack()
speaker_status_label.pack()


def read_from_arduino():
    global microphone, sound, alert, temperature_values
    while True:
        line = ser.readline().decode('utf-8').strip()
        if line:
            # Αναμενόμενη μορφή: TEMP:27.4;MIC:1;SOUND:0;ALERT:0
            try:
                parts = line.split(';')
                data = {}
                for part in parts:
                    key, val = part.split(':')
                    data[key] = val

                temp = float(data.get('TEMP', 0))
                mic_status = data.get('MIC', '0') == '1'
                sound_status = data.get('SOUND', '0') == '1'
                alert_status = data.get('ALERT', '0') == '1'

                temperature_values.append(temp)

                window.after(0, update_ui, temp, mic_status, sound_status, alert_status)

                global start_time
                if time.time() - start_time >= 10:
                    avg = sum(temperature_values) / len(temperature_values)
                    window.after(0, update_avg_temp, avg)
                    temperature_values.clear()
                    start_time = time.time()

            except Exception as e:
                print("Parsing error:", e)


def update_ui(temp, mic_status, sound_status, alert_status):
    temp_value_label.config(text=f"{temp:.1f} °C")

    mic_icon.config(image=mic_on_img if mic_status else mic_off_img)
    mic_status_label.config(
        text="ΕΝΕΡΓΟ" if mic_status else "ΑΝΕΝΕΡΓΟ",
        fg="lime" if mic_status else "gray"
    )

    if alert_status:
        speaker_icon.config(image=alarm_img)
        speaker_status_label.config(text="VEHICLE APPROACHES", fg="red")
    else:
        speaker_icon.config(image=speaker_on_img if sound_status else speaker_off_img)
        speaker_status_label.config(
            text="ΕΝΕΡΓΟ" if sound_status else "ΑΝΕΝΕΡΓΟ",
            fg="lime" if sound_status else "gray"
        )


def update_avg_temp(avg):
    temp_avg_label.config(text=f"Μ.Ο.: {avg:.1f} °C")


threading.Thread(target=read_from_arduino, daemon=True).start()
window.mainloop()
