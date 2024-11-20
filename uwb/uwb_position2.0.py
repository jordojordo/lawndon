import time
import turtle
import cmath
import socket
import json
import threading
import numpy as np

# Constants
DISTANCE_ANCHOR = 18.4
METER_TO_PIXEL = 22

ANCHOR_IDS = ["1786", "1783"]  # Extendable for more anchors

# Shared data between threads
uwb_list = []
data_lock = threading.Lock()

# Turtle draw functions ################################################


def screen_init(width=1200, height=800, t=turtle):
    t.setup(width, height)
    t.tracer(False)
    t.hideturtle()
    t.speed(0)


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


def draw_ui(t):
    write_txt(-300, 250, "UWB Position", "black", t, f=("Arial", 32, "normal"))
    fill_rect(-400, 200, 800, 40, "black", t)
    write_txt(-50, 205, "WALL", "yellow", t, f=("Arial", 24, "normal"))


def draw_uwb_anchor(x, y, txt, range, t):
    r = 20
    fill_cycle(x, y, r, "green", t)
    write_txt(
        x + r, y, txt + ": " + str(range) + "M", "black", t, f=("Arial", 16, "normal")
    )


def draw_uwb_tag(x, y, txt, t):
    pos_x = -250 + int(x * METER_TO_PIXEL)
    pos_y = 150 - int(y * METER_TO_PIXEL)
    r = 20
    fill_cycle(pos_x, pos_y, r, "blue", t)
    write_txt(
        pos_x,
        pos_y,
        txt + ": (" + str(x) + ", " + str(y) + ")",
        "black",
        t,
        f=("Arial", 16, "normal"),
    )


# UDP UWB data analysis ################################################


def tag_pos(anchors, distances):
    if len(anchors) < 2:
        print("Error: At least two anchors are required.")
        return -1, -1
    try:
        # Convert anchor positions and distances to numpy arrays
        A = np.array(anchors)
        b = np.array(distances)

        # Calculate the position using least squares
        A_transpose = A.T
        A_pseudo_inverse = np.linalg.pinv(A_transpose @ A) @ A_transpose
        position = A_pseudo_inverse @ b

        return round(position[0], 1), round(position[1], 1)
    except Exception as e:
        print(f"Error calculating position data: {e}")
        return -1, -1


def uwb_range_offset(uwb_range):
    # Apply any range offset if needed
    return uwb_range


# Socket handling in a separate thread #################################


def socket_listener():
    global uwb_list

    UDP_PORT = 8080
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(("0.0.0.0", UDP_PORT))
    sock.listen(1)

    print(f"Listening on port {UDP_PORT} for incoming connections...")

    conn, addr = sock.accept()
    print(f"Connected by {addr}")

    while True:
        try:
            line = conn.recv(1024).decode("UTF-8")

            if not line:
                print("Connection closed by client")
                break  # Connection closed

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


# Main function ########################################################


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

    draw_ui(t_ui)

    anchor_positions = [
        (-250, 150),
        (-250 + METER_TO_PIXEL * DISTANCE_ANCHOR, 150),
    ]  # Extendable for more anchors
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

        if node_count >= 2:
            x, y = tag_pos(anchor_positions, anchor_ranges)
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
