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
#arduino = serial.Serial(port='COM12', baudrate=9600, timeout=.1)
#ser = arduino

# Windows
window = tk.Tk()
window.title("Helmetino")
window.geometry("600x400")
window.configure(bg="#e5e0d9")

# Image initialization
led_on_img = ImageTk.PhotoImage(Image.open("./images/led_on.png").resize((80, 80)))
led_off_img = ImageTk.PhotoImage(Image.open("./images/led_off.png").resize((60, 60)))
speaker_on_img = ImageTk.PhotoImage(Image.open("./images/sound.png").resize((60, 60)))
speaker_off_img = ImageTk.PhotoImage(Image.open("./images/sound_mute.png").resize((60, 60)))
alarm_img = ImageTk.PhotoImage(Image.open("./images/alarm.png").resize((60, 60)))
cyclist_img = ImageTk.PhotoImage(Image.open("./images/bike.png").resize((60, 60)))
map_img = ImageTk.PhotoImage(Image.open("./images/map.png").resize((500, 600)))
logo_img = ImageTk.PhotoImage(Image.open("./images/logo.png").resize((200, 200)))

logo_label = tk.Label(window, image=logo_img, bg="#e5e0d9")
logo_label.pack(pady=(10, 0))

# Helmetino
#title1 = tk.Label(window, text="Helmetino", font=("Helvetica", 28, "bold"), bg="#e5e0d9", fg="black")
title2 = tk.Label(window, text="Rider ID: 7008797\n", font=("Helvetica", 15, "bold"), bg="#e5e0d9", fg="black")
#title1.pack(pady=5)
title2.pack(pady=10)

# Layout structure 

# Main horizontal layout frame
main_frame = tk.Frame(window, bg="#e5e0d9")
main_frame.pack(fill="both", expand=True, padx=10)

# Left panel (temp + led)
left_frame = tk.Frame(main_frame, bg="#e5e0d9")
left_frame.grid(row=0, column=0, sticky="n")

# Map panel (center)
map_canvas = tk.Canvas(main_frame, width=500, height=550, bg="white", highlightthickness=0)
map_canvas.grid(row=0, column=1, padx=10)

map_canvas.create_image(0, 0, anchor="nw", image=map_img)

# Right panel (sound + fallen)
right_frame = tk.Frame(main_frame, bg="#e5e0d9")
right_frame.grid(row=0, column=2, sticky="n")


# Create a cyclist marker
cyclist_marker = map_canvas.create_oval(10, 10, 20, 20, fill="black")

# Create second cyclist marker (for nearby detection)
nearby_marker = map_canvas.create_oval(10, 10, 20, 20, fill="red", state='hidden')


# Trail's coordinates
trail_coords = [
    (277, 342), (277, 342), (285, 337), (295, 334), (305, 330), (313, 326), (321, 320), (327, 315), (332, 313), (335, 310),
    (339, 300), (345, 296), (344, 288), (343, 282), (341, 278), (339, 273), (337, 270), (333, 266), (331, 262), (325, 253),
    (323, 248), (319, 241), (318, 232), (319, 226), (319, 222), (325, 218), (325, 218), (330, 216), (333, 214), (337, 210),
    (341, 207), (345, 206), (347, 203), (350, 203), (352, 202), (352, 197), (353, 192), (354, 187), (356, 183), (359, 180),
    (359, 174), (360, 172), (360, 166), (359, 163), (355, 154), (351, 147), (348, 142), (345, 136), (343, 132), (340, 130),
    (336, 125), (333, 120), (327, 112), (321, 110), (319, 108), (316, 107), (312, 105), (309, 102), (307, 101), (303, 97),
    (296, 91), (293, 90), (291, 88), (287, 86), (281, 81), (278, 80), (273, 78), (267, 73), (260, 70), (253, 70), (248, 70),
    (239, 71), (237, 71), (232, 71), (227, 71), (223, 74), (220, 74), (215, 74), (208, 74), (203, 75), (200, 74), (199, 80),
    (196, 83), (193, 86), (193, 86), (192, 89), (189, 94), (189, 94), (186, 98), (183, 98), (184, 107), (183, 109), (183, 114),
    (183, 122), (181, 126), (179, 132), (177, 134), (175, 137), (169, 142), (164, 145), (159, 149), (156, 151), (153, 155),
    (147, 156), (139, 153), (137, 158), (131, 162), (127, 166), (121, 170), (115, 174), (112, 178), (111, 182), (110, 188),
    (108, 196), (108, 203), (109, 206), (109, 206), (110, 212), (111, 220), (113, 225), (117, 230), (123, 235), (127, 242),
    (129, 245), (129, 245), (131, 248), (134, 252), (137, 257), (140, 258), (142, 260), (141, 264), (143, 270), (147, 274),
    (152, 280), (157, 284), (160, 289), (163, 294), (166, 298), (169, 301), (174, 304), (179, 307), (183, 312), (187, 315),
    (191, 319), (194, 320), (199, 322), (205, 326), (207, 326), (211, 326), (212, 327), (220, 330), (223, 332), (227, 334),
    (231, 336), (238, 340), (238, 344), (239, 347), (241, 353), (241, 355), (241, 355), (242, 360), (242, 368), (242, 373),
    (242, 380), (242, 382), (241, 388), (239, 392), (239, 396), (238, 401), (236, 401), (231, 406), (227, 412), (224, 412),
    (221, 417), (219, 418), (215, 420), (212, 426), (209, 430), (207, 435), (205, 438), (205, 444), (206, 450), (209, 453),
    (212, 455), (218, 460), (221, 461), (226, 464), (236, 469), (242, 473), (245, 476), (247, 479), (250, 483), (253, 488),
    (254, 490), (255, 492),
]
current_index = 0

def update_location():
    global current_index
    if current_index < len(trail_coords):
        x, y = trail_coords[current_index]
        map_canvas.coords(cyclist_marker, x, y, x + 10, y + 10)
        map_canvas.coords(nearby_marker, x, y, x + 10, y + 10)  
        current_index += 1
        window.after(500, update_location)

update_location()

# Temperature box
temp_canvas = tk.Canvas(left_frame, width=150, height=150, bg="#e5e0d9", highlightthickness=0)
temp_canvas.pack(pady=10)

circle = temp_canvas.create_oval(10, 10, 140, 140, fill="#0077BA", outline="")

temp_value_label = temp_canvas.create_text(75, 60, text="-- °C", font=("Helvetica", 20), fill="#e5e0d9")
temp_avg_label = temp_canvas.create_text(75, 100, text="Μ.Ο.: -- °C", font=("Helvetica", 12), fill="gray")

# Led box
led_canvas = tk.Canvas(left_frame, width=150, height=150, bg="#e5e0d9", highlightthickness=0)
led_canvas.pack(pady=10)

led_circle = led_canvas.create_oval(10, 10, 140, 140, fill="#0077BA", outline="")

led_icon = led_canvas.create_image(75, 68, image=led_off_img)

led_status_label = led_canvas.create_text(75, 110, text="Led is OFF", font=("Helvetica", 11, "bold"), fill="#e5e0d9")


# Sound box / alert
speaker_canvas = tk.Canvas(right_frame, width=150, height=150, bg="#e5e0d9", highlightthickness=0)
speaker_canvas.pack(pady=10)

speaker_circle = speaker_canvas.create_oval(10, 10, 140, 140, fill="#0077BA", outline="")

speaker_icon = speaker_canvas.create_image(75, 66, image=speaker_off_img)

speaker_status_label = speaker_canvas.create_text(75, 110, text="Sound is OFF", font=("Helvetica", 11, "bold"), fill="#e5e0d9")

# Fallen
fallen_canvas = tk.Canvas(right_frame, width=150, height=150, bg="#e5e0d9", highlightthickness=0)
fallen_canvas.pack(pady=10)

fallen_circle = fallen_canvas.create_oval(10, 10, 140, 140, fill="#0077BA", outline="")

fallen_icon = fallen_canvas.create_image(75, 60, image=cyclist_img)

fallen_status_label = fallen_canvas.create_text(75, 108, text="The cyclist\n is on track", font=("Helvetica", 11, "bold"), fill="#e5e0d9")

#fallen_icon.pack()
#fallen_status_label.pack()
fallen_state = False

def read_from_arduino():
    global led, sound, alert, temperature_values
    data = {}  # Buffer για την αποθήκευση τιμών
    required_keys = {"TEMP", "LIGHT", "SOUND", "CRASH", "NEARBY"}

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
                    nearby_status = data.get("NEARBY") == "1"

                    temperature_values.append(temp)

                    window.after(0, update_ui, temp, led_status, sound_status, fallen_status, nearby_status)

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

def update_ui(temp, led_status, sound_status, fallen_status, nearby_status):
    print(f"Temp: {temp:.1f} °C, LED: {led_status}, Sound: {sound_status}, Fallen: {fallen_status}")
    temp_value_label.config(text=f"{temp:.1f} °C")

    led_icon.config(image=led_on_img if led_status else led_off_img)
    led_status_label.config(
        text="Led is ON" if led_status else "Led is OFF",
        fg="lime" if led_status else "#e5e0d9"
    )


    speaker_icon.config(image=speaker_on_img if sound_status else speaker_off_img)
    speaker_status_label.config(
        text="Sound is ON" if sound_status else "Sound is OFF",
        fg="lime" if sound_status else "#e5e0d9"
    )

    # Update fallen label
    if fallen_status:
        fallen_icon.config(image=alarm_img)
        fallen_status_label.config(text="The cyclist has crushed", fg="red")
    else:
        fallen_status_label.config(text="The cyclist is on track", fg="#e5e0d9")

    # Update nearby cyclist marker
    if nearby_status:
        map_canvas.itemconfigure(nearby_marker, state='normal')
    else:
        map_canvas.itemconfigure(nearby_marker, state='hidden')

def update_avg_temp(avg):
    temp_avg_label.config(text=f"Μ.Ο.: {avg:.1f} °C")


#threading.Thread(target=read_from_arduino, daemon=True).start()
window.mainloop()
