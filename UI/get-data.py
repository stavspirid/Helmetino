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
window.geometry("840x1000")
window.configure(bg="#e5e0d9")

# Image initialization
led_on_img = ImageTk.PhotoImage(Image.open("./images/led_on.png").resize((65, 65)))
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

# Warning label for nearby cyclist
warning_label = map_canvas.create_text(
    400, 30, 
    text="⚠️ Warning: Another cyclist is nearby",
    font=("Helvetica", 12, "bold"),
    fill="red",
    state='hidden',  # Initially hidden
    anchor='ne' 
)

# Right panel (sound + fallen)
right_frame = tk.Frame(main_frame, bg="#e5e0d9")
right_frame.grid(row=0, column=2, sticky="n")

# Create a cyclist marker
cyclist_marker = map_canvas.create_oval(10, 10, 20, 20, fill="black")

# Create second cyclist marker (for nearby detection)
nearby_marker = map_canvas.create_oval(10, 10, 20, 20, fill="red", state='hidden')


# Trail's coordinates
# Trail's coordinates (short form)
trail_coords = [
    (276, 341), (282, 338), (287, 335), (291, 333), (297, 331), (304, 329), (309, 327), (315, 325), (319, 321), (323, 318),
    (326, 313), (329, 307), (332, 303), (335, 299), (336, 293), (337, 286), (337, 281), (337, 275), (336, 270), (332, 265),
    (330, 261), (327, 257), (324, 253), (321, 249), (316, 242), (315, 238), (315, 231), (316, 225), (321, 221), (324, 216),
    (328, 212), (332, 210), (337, 207), (340, 203), (342, 197), (347, 195), (350, 190), (352, 185), (355, 177), (356, 169),
    (357, 162), (355, 157), (354, 152), (351, 146), (349, 141), (346, 136), (342, 131), (338, 127), (333, 123), (329, 119),
    (323, 113), (317, 108), (312, 105), (308, 102), (302, 98), (298, 95), (293, 92), (289, 88), (284, 84), (279, 81),
    (273, 78), (267, 74), (259, 74), (252, 73), (246, 73), (241, 72), (234, 71), (228, 72), (219, 69), (212, 69),
    (205, 71), (199, 76), (195, 78), (190, 83), (186, 88), (181, 93), (179, 101), (177, 107), (177, 112), (173, 119),
    (171, 124), (166, 132), (161, 137), (153, 139), (146, 143), (137, 148), (126, 151), (119, 157), (112, 163), (109, 168),
    (103, 171), (101, 177), (98, 184), (97, 189), (96, 196), (96, 199), (96, 205), (96, 212), (97, 218), (99, 225),
    (102, 230), (106, 236), (108, 239), (112, 239), (114, 245), (117, 248), (121, 253), (125, 258), (129, 262), (134, 266),
    (139, 271), (143, 275), (147, 282), (150, 285), (156, 292), (159, 297), (162, 299), (165, 306), (172, 310), (176, 314),
    (185, 319), (193, 323), (199, 327), (205, 329), (212, 333), (217, 335), (223, 336), (229, 338), (233, 343), (235, 347),
    (238, 355), (236, 359), (237, 367), (237, 377), (238, 382), (237, 389), (234, 398), (233, 408), (230, 410), (223, 414),
    (216, 421), (212, 424), (204, 430), (198, 437), (197, 447), (199, 457), (204, 460), (208, 462), (215, 466), (224, 470),
    (233, 471), (240, 476), (243, 478), (246, 482), (248, 485), (249, 489), (251, 491),
]
current_index = 0

def update_location():
    global current_index
    if current_index < len(trail_coords):
        x, y = trail_coords[current_index]
        map_canvas.coords(cyclist_marker, x, y, x + 10, y + 10)
        map_canvas.coords(nearby_marker, x + 20, y, x + 10, y + 10)  
        current_index += 1
        window.after(2000, update_location)

update_location()

# Temperature box
temp_canvas = tk.Canvas(left_frame, width=150, height=150, bg="#e5e0d9", highlightthickness=0)
temp_canvas.pack(pady=10)

circle = temp_canvas.create_oval(10, 10, 140, 140, fill="#0077BA", outline="")

temp_value_label = temp_canvas.create_text(75, 60, text="-- °C", font=("Helvetica", 20, "bold"), fill="#e5e0d9")
temp_avg_label = temp_canvas.create_text(75, 100, text="Μ.Ο.: -- °C", font=("Helvetica", 12, "bold"), fill="black")

# Led box
led_canvas = tk.Canvas(left_frame, width=150, height=150, bg="#e5e0d9", highlightthickness=0)
led_canvas.pack(pady=10)

led_circle = led_canvas.create_oval(10, 10, 140, 140, fill="#0077BA", outline="")

led_icon = led_canvas.create_image(75, 68, image=led_off_img)

led_status_label = led_canvas.create_text(75, 110, text="Led: OFF", font=("Helvetica", 11, "bold"), fill="#e5e0d9")


# Sound box / alert
speaker_canvas = tk.Canvas(right_frame, width=150, height=150, bg="#e5e0d9", highlightthickness=0)
speaker_canvas.pack(pady=10)

speaker_circle = speaker_canvas.create_oval(10, 10, 140, 140, fill="#0077BA", outline="")

speaker_icon = speaker_canvas.create_image(75, 66, image=speaker_off_img)

speaker_status_label = speaker_canvas.create_text(75, 110, text="Sound: OFF", font=("Helvetica", 11, "bold"), fill="#e5e0d9")

# Fallen
fallen_canvas = tk.Canvas(right_frame, width=150, height=150, bg="#e5e0d9", highlightthickness=0)
fallen_canvas.pack(pady=10)

fallen_circle = fallen_canvas.create_oval(10, 10, 140, 140, fill="#0077BA", outline="")

fallen_icon = fallen_canvas.create_image(75, 60, image=cyclist_img)

fallen_status_label = fallen_canvas.create_text(75, 108, text="    Status:\nON TRACK", font=("Helvetica", 11, "bold"), fill="#e5e0d9")


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
    print(f"Temp: {temp:.1f} °C, LED: {led_status}, Sound: {sound_status}, Fallen: {fallen_status}, Nearby: {nearby_status}")

    temp_canvas.itemconfig(temp_value_label, text=f"{temp:.1f} °C")

    led_canvas.itemconfig(led_icon, image=led_on_img if led_status else led_off_img)
    led_canvas.itemconfig(led_status_label, text="Led: ON" if led_status else "Led: OFF", fill="#e5e0d9" if led_status else "#e5e0d9")

    speaker_canvas.itemconfig(speaker_icon, image=speaker_on_img if sound_status else speaker_off_img)
    speaker_canvas.itemconfig(speaker_status_label, text="Sound: ON" if sound_status else "Sound: OFF", fill="#e5e0d9" if sound_status else "#e5e0d9")

    if fallen_status:
        fallen_canvas.itemconfig(fallen_icon, image=alarm_img)
        fallen_canvas.itemconfig(fallen_status_label, text=" Status:\nCRASH", fill="#d81111")
    else:
        fallen_canvas.itemconfig(fallen_status_label, text="    Status:\nON TRACK", fill="#e5e0d9")
        fallen_canvas.itemconfig(fallen_icon, image=cyclist_img)

    if nearby_status:
        map_canvas.itemconfigure(nearby_marker, state='normal')
        map_canvas.itemconfigure(warning_label, state='normal')
    else:
        map_canvas.itemconfigure(nearby_marker, state='hidden')
        map_canvas.itemconfigure(warning_label, state='hidden')


def update_avg_temp(avg):
    temp_canvas.itemconfig(temp_avg_label, text=f"Μ.Ο.: {avg:.1f} °C")


#threading.Thread(target=read_from_arduino, daemon=True).start()
window.mainloop()