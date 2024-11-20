import time
import turtle
import cmath
import socket
import json
import threading
import numpy as np
import math

# Constants
DISTANCE_ANCHOR = 18.4  # in meters
METER_TO_PIXEL = 22

ANCHOR_IDS = ["50", "51", "52"]

# Shared data between threads
uwb_list = []
data_lock = threading.Lock()

def screen_init(width=1200, height=800, t=turtle):
    t.setup(width, height)
    t.tracer(False)
    t.hideturtle()
    t.speed(0)

    # Set world coordinates to match the area of interest
    t.setworldcoordinates(
        -5 * METER_TO_PIXEL,
        -5 * METER_TO_PIXEL,
        (DISTANCE_ANCHOR + 5) * METER_TO_PIXEL,
        (DISTANCE_ANCHOR * math.sqrt(3) / 2 + 5) * METER_TO_PIXEL,
    )


def turtle_init(t=turtle):
    t.hideturtle()
    t.speed(0)


def draw_line(x0, y0, x1, y1, color="black", t=turtle):
    t.pencolor(color)
    t.up()
    t.goto(x0, y0)
    t.down()
    t.goto(x1, y1)
    t.up()


def draw_fastU(x, y, length, color="black", t=turtle):
    draw_line(x, y, x, y + length, color, t)


def draw_fastV(x, y, length, color="black", t=turtle):
    draw_line(x, y, x + length, y, color, t)


def draw_cycle(x, y, r, color="black", t=turtle):
    t.pencolor(color)
    t.up()
    t.goto(x, y - r)
    t.setheading(0)
    t.down()
    t.circle(r)
    t.up()


def fill_cycle(x, y, r, color="black", t=turtle):
    t.up()
    t.goto(x, y)
    t.down()
    t.dot(r, color)
    t.up()


def write_txt(x, y, txt, color="black", t=turtle, f=("Arial", 12, "normal")):
    t.pencolor(color)
    t.up()
    t.goto(x, y)
    t.down()
    t.write(txt, move=False, align="left", font=f)
    t.up()


def draw_rect(x, y, w, h, color="black", t=turtle):
    t.pencolor(color)
    t.up()
    t.goto(x, y)
    t.down()
    t.goto(x + w, y)
    t.goto(x + w, y + h)
    t.goto(x, y + h)
    t.goto(x, y)
    t.up()


def fill_rect(x, y, w, h, color=("black", "black"), t=turtle):
    t.begin_fill()
    draw_rect(x, y, w, h, color, t)
    t.end_fill()


def clean(t=turtle):
    t.clear()


def draw_uwb_anchor(x_m, y_m, txt, range, t):
    x = x_m * METER_TO_PIXEL
    y = y_m * METER_TO_PIXEL
    r = 20
    fill_cycle(x, y, r, "green", t)
    write_txt(
        x + r, y, txt + ": " + str(range) + "M", "black", t, f=("Arial", 16, "normal")
    )


def draw_uwb_tag(x_m, y_m, txt, t):
    x = x_m * METER_TO_PIXEL
    y = y_m * METER_TO_PIXEL
    r = 20
    fill_cycle(x, y, r, "blue", t)
    write_txt(
        x,
        y,
        txt + ": (" + str(round(x_m, 2)) + ", " + str(round(y_m, 2)) + ")",
        "black",
        t,
        f=("Arial", 16, "normal"),
    )


def tag_pos(positions, distances):
    if len(positions) < 3:
        print("Error: At least three anchors are required.")
        return -1, -1
    try:
        x1, y1 = positions[0]
        x2, y2 = positions[1]
        x3, y3 = positions[2]
        r1 = distances[0]
        r2 = distances[1]
        r3 = distances[2]

        # Formulate matrices
        A = np.array([
            [x2 - x1, y2 - y1],
            [x3 - x1, y3 - y1]
        ])
        B = np.array([
            0.5 * ((x2**2 - x1**2 + y2**2 - y1**2) - (r2**2 - r1**2)),
            0.5 * ((x3**2 - x1**2 + y3**2 - y1**2) - (r3**2 - r1**2))
        ])

        # Solve for x and y
        position = np.linalg.lstsq(A, B, rcond=None)[0]

        return round(position[0], 2), round(position[1], 2)
    except Exception as e:
        print(f"Error calculating position data: {e}")
        return -1, -1


def uwb_range_offset(uwb_range):
    return uwb_range


def socket_listener():
    global uwb_list

    TCP_PORT = 8080
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(("0.0.0.0", TCP_PORT))
    sock.listen(1)

    print(f"Listening on port {TCP_PORT} for incoming connections...")

    conn, addr = sock.accept()
    print(f"Connected by {addr}")
    buffer = ''

    while True:
        try:
            data = conn.recv(1024).decode("UTF-8")
            if not data:
                print("Connection closed by client")
                break  # Connection closed

            buffer += data
            while '\n' in buffer:
                line, buffer = buffer.split('\n', 1)
                if line:
                    uwb_data = json.loads(line)
                    with data_lock:
                        uwb_list = uwb_data.get("links", [])
                    for uwb_anchor in uwb_list:
                        print(f"anchor: {uwb_anchor}")
        except json.JSONDecodeError:
            print("Received invalid JSON data.")
        except Exception as e:
            print(f"Error reading data: {e}")
            break
    conn.close()

def main():
    # Start the socket listener in a separate thread
    threading.Thread(target=socket_listener, daemon=True).start()

    # Initialize the Turtle screen
    screen_init()

    # Turtle instances
    t_ui = turtle.Turtle()
    t_anchors = [turtle.Turtle() for _ in ANCHOR_IDS]
    t_tag = turtle.Turtle()
    turtle_init(t_ui)
    for t in t_anchors:
        turtle_init(t)
    turtle_init(t_tag)

    # Anchor positions in meters
    anchor_positions = [
        (0.0, 0.0),  # Anchor 50
        (DISTANCE_ANCHOR, 0.0),  # Anchor 51
        (DISTANCE_ANCHOR / 2, DISTANCE_ANCHOR * math.sqrt(3) / 2),  # Anchor 52
    ]
    anchor_ranges = [0.0] * len(ANCHOR_IDS)

    def update():
        nonlocal anchor_ranges
        node_count = 0
        with data_lock:
            local_uwb_list = uwb_list.copy()

        for one in local_uwb_list:
            if one.get("A") in ANCHOR_IDS:
                index = ANCHOR_IDS.index(one.get("A"))
                clean(t_anchors[index])
                anchor_ranges[index] = uwb_range_offset(float(one.get("R", 0)))
                draw_uwb_anchor(
                    anchor_positions[index][0],
                    anchor_positions[index][1],
                    f"A{ANCHOR_IDS[index]}",
                    anchor_ranges[index],
                    t_anchors[index],
                )
                node_count += 1

        if node_count >= 3:
            # Build positions and distances for anchors with valid ranges
            valid_positions = []
            valid_distances = []
            for i in range(len(anchor_ranges)):
                if anchor_ranges[i] > 0:
                    valid_positions.append(anchor_positions[i])
                    valid_distances.append(anchor_ranges[i])

            if len(valid_positions) >= 3:
                x, y = tag_pos(valid_positions, valid_distances)
                if x != -1:
                    print(f"Tag Position: x={x}, y={y}")
                    clean(t_tag)
                    draw_uwb_tag(x, y, "TAG", t_tag)

        turtle.update()
        # Schedule the next update after 100 milliseconds
        turtle.ontimer(update, 100)

    # Start the update loop
    update()

    turtle.mainloop()


if __name__ == "__main__":
    main()
