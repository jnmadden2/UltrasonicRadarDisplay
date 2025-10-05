import pyqtgraph as pg
from pyqtgraph.Qt import QtWidgets, QtCore
import numpy as np
import serial
from typing import Tuple

import paho.mqtt.client as mqtt
import urllib.request
import webbrowser
import socket

# Define a global flag to determine data source
READ_FROM_FILE = True  # Set to False if reading from MCU

# ---- Serial Port Configuration ----
SERIAL_PORT = "COM4"  # Change this to match your MCU's port (e.g., "/dev/ttyUSB0" for Linux)
BAUD_RATE = 115200     # Match your MCU's baud rate
maxAge = 10 #maximum number of points displayed at once

is_reading = False  # Global flag to indicate if read_from_serial is running

ip_address = "10."  # Replace with the RPI IP, this will be 10.xxx.xxx.xxx
broker_port = 1883  # Default MQTT port
topic = "esp32/sensor1"  # keep as esp32/sensor1
mystr = ""
replaceText = ""  # this will replace what the .html file has and what the server displays



color_pool = [
    (255, 0, 0),       # Red
    (0, 255, 0),       # Green
    (0, 0, 255),       # Blue
    (255, 165, 0),     # Orange
    (255, 255, 255),   # White
    (255, 255, 0),     # Yellow
    (128, 0, 128),     # Purple
    (0, 255, 255),     # Cyan
    (255, 192, 203),   # Pink
    #(128, 128, 128),   # Gray
    (0, 128, 0),       # Dark Green
    (0, 0, 128),       # Navy
    (128, 0, 0),       # Maroon
    (255, 105, 180),   # Hot Pink
    (173, 216, 230),   # Light Blue
]

available_colors = color_pool.copy()

def open_url_by_ip():
    try:
        socket.inet_aton(ip_address) # Validate the IP address
        url = f"http://{ip_address}"
        fp = urllib.request.urlopen(url)
        mybytes = fp.read()

        mystr = mybytes.decode("utf8")
        fp.close()

        print(mystr)
    except socket.error:
        print("Invalid IP address.")
    except webbrowser.Error:
       print("Browser couldn't be opened. Make sure you have a default web browser set up.")
    client.publish(topic, replaceText)

def get_color():
    # print(len(available_colors))
    # print("\n")
    # print(available_colors)
    # print("\n")
    return available_colors.pop(0) if available_colors else (128, 128, 128)

def release_color(color):
    if color == (128,128,128):
        return
    
    available_colors.append(color)

class Cluster:
    def __init__(self, id: int, color: Tuple[int, int, int], initial_point: Tuple[float, float]):
        self.id = id
        self.color = color
        #self.points = [initial_point]
        self.points = {initial_point: 1}
        self.min_x = self.max_x = initial_point[0]
        self.min_y = self.max_y = initial_point[1]

    def addPoint(self, point):
        # self.points.append(point)
        # x, y = point
        # self.min_x = min(self.min_x, x)
        # self.max_x = max(self.max_x, x)
        # self.min_y = min(self.min_y, y)
        # self.max_y = max(self.max_y, y)

        if point in self.points:
            self.points[point] += 1
        else:
            self.points[point] = 1
            x, y = point
            self.min_x = min(self.min_x, x)
            self.max_x = max(self.max_x, x)
            self.min_y = min(self.min_y, y)
            self.max_y = max(self.max_y, y)

    def compatibility(self, point, threshold = 10):
        x,y = point
        return (
            self.min_x - threshold <= x <= self.max_x + threshold and self.min_y - threshold <= y <= self.max_y + threshold
        )
    
    def removePoint(self, point):
        if point in self.points:
            self.points[point] -= 1
            if self.points[point] == 0:
                del self.points[point]
                if len(self.points) == 0:
                    return True
        return False
    
class ClusterManager:
    clusters = {}
    points2Cluster = {}
    max_index = maxAge
    cur_cluster_id = 0

    @staticmethod
    def getCluster(id: int):
        return ClusterManager.clusters.get(id)
    
    @staticmethod
    def addCluster(cluster: Cluster):
        ClusterManager.clusters[cluster.id] = cluster
    
    @staticmethod
    def removeCluster(id: int):
        if id in ClusterManager.clusters:
            release_color(ClusterManager.clusters[id].color)
            del ClusterManager.clusters[id]

    @staticmethod
    def updateId():
        ClusterManager.cur_cluster_id += 1
        
        if ClusterManager.cur_cluster_id > maxAge:
            ClusterManager.cur_cluster_id = 0

    @staticmethod
    def processNewPoint(point: Tuple[float, float]):
        for cluster_id in ClusterManager.clusters:
            cluster = ClusterManager.getCluster(cluster_id)
            if cluster.compatibility(point):
                cluster.addPoint(point)
                ClusterManager.points2Cluster[point] = cluster.id
                return

        newCluster = Cluster(ClusterManager.cur_cluster_id, get_color(), point)
        #print("\n")
        #print (newCluster.color)
        ClusterManager.addCluster(newCluster)  # Make sure to add the new cluster
        ClusterManager.points2Cluster[point] = newCluster.id  # Associate point with new cluster
        ClusterManager.updateId()

    @staticmethod
    def removePoint(point: Tuple[float, float]):
        cluster = ClusterManager.getCluster(ClusterManager.points2Cluster[point])
        delete = cluster.removePoint(point)

        if delete:
            ClusterManager.removeCluster(cluster.id)
            del ClusterManager.points2Cluster[point]

    @staticmethod
    def findPointColor(point: Tuple[float, float]):
        cluster = ClusterManager.getCluster(ClusterManager.points2Cluster[point])
        if cluster is None:
            return None  # Or a default color
        
        #cluster = ClusterManager.getCluster(cluster_id)
        return cluster.color


if not READ_FROM_FILE:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    ser.reset_input_buffer()  # Clear any existing data in the buffern

app = QtWidgets.QApplication([])
win = pg.GraphicsLayoutWidget(title="Object Detector Display Sweep")
win.showFullScreen()

plot = win.addPlot()
plot.setAspectLocked(True) #intended to may x and y change the same
#plot.plot([1], [1], pen=None, symbol='o', symbolBrush='r', symbolSize=100)

plot.hideAxis('bottom')
plot.hideAxis('left')

#color_pool = [(255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 0)]  # Available clusters Red, Green, Blue, Yellow 

#restrictions and intervals
max_distance = 1000 #max of 1 meter (1000mm)
angle_steps = 15  #degree increments
big_distance_increment = 100 #increment by 10 cm for semi circles

distance_steps = 100  # mm steps (each semi-circle indicates 10 cm)

# Draw concentric distance semi-circles (each representing 10 cm)
angles = np.linspace(0, np.pi, 120)  # Angles from 0 to 180 degrees
text_items = []  # Store labels for dynamic scaling


# Function to adjust text size based on zoom level
def update_text_size():
    view_range = plot.viewRange()
    x_range = view_range[0][1] - view_range[0][0]
    font_size = max(8, int(30 * (1000 / x_range)))  # Adjust multiplier for scaling
    for text_item, label_text in text_items:
        text_item.setHtml(f'<div style="font-size: {font_size}px; color: white;">{label_text}</div>')

#draws 10 cm increments
for r in range(distance_steps, max_distance + distance_steps, distance_steps):
    x = r * np.cos(angles)
    y = r * np.sin(angles)
    plot.plot(x, y, pen=pg.mkPen((255, 255, 255), width=1))

    # Add distance label in cm at the bottom (x = r, y = 0)
    distance_cm = r / 10  # Convert mm to cm
    right_label = pg.TextItem(anchor=(0.5, 0))
    right_label.setHtml(f'<div style="font-size: 12px; color: white;">{distance_cm} cm</div>')
    right_label.setPos(r, 0)  # Position at (r, 0) along the x-axis
    plot.addItem(right_label)
    text_items.append((right_label, f'{distance_cm} cm'))

    # Add distance label on the left side (x = -r, y = 0)
    left_label = pg.TextItem(anchor=(0.5, 0))
    left_label.setHtml(f'<div style="font-size: 12px; color: white;">{distance_cm} cm</div>')
    left_label.setPos(-r, 0)
    plot.addItem(left_label)
    text_items.append((left_label, f'{distance_cm} cm'))

# Draw radial lines for angles
angles_deg = np.arange(0, 181, angle_steps)
label_distance = max_distance + 50  # Distance slightly outside the largest arc
for angle in angles_deg:
    x = [0, max_distance * np.cos(np.radians(angle))]
    y = [0, max_distance * np.sin(np.radians(angle))]
    plot.plot(x, y, pen=pg.mkPen((255, 255, 0), width=1))


    label_x = label_distance * np.cos(np.radians(angle))
    label_y = label_distance * np.sin(np.radians(angle))

    # Flip angle for readability when upside-down
    rotation_angle = -angle + 90

    #text = pg.TextItem(f"{angle}°", color='red', anchor=(0.5, 0.5))
    #right_label.setHtml(f'<div style="font-size: 12px; color: white;">{distance_cm} cm</div>')
    text = pg.TextItem(anchor=(0.5, 0.5))
    text.setHtml(f'<div style="font-size: 12px; color: white;">{angle}°</div>')
    text.setPos(label_x, label_y)
    text.setRotation(rotation_angle)
    plot.addItem(text)
    text_items.append((text, f'{angle}°'))
# Connect zoom/scroll events to update the font size
plot.sigRangeChanged.connect(lambda: update_text_size())

# Initial text size adjustment
update_text_size()

# Cursor tracking: Display angle and distance based on mouse position
cursor_label = pg.TextItem(color=(255, 255, 0))
plot.addItem(cursor_label)

def mouse_moved(evt):
    pos = evt[0]  # using signal proxy turns original arguments into a tuple
    if plot.sceneBoundingRect().contains(pos):
        mouse_point = plot.vb.mapSceneToView(pos)
        x, y = mouse_point.x(), mouse_point.y()
        distance = np.sqrt(x**2 + y**2)
        angle = np.degrees(np.arctan2(y, x))
        if angle < 0:
            angle += 360
        if 0 <= angle <= 180 and distance <= (1000 + 10) and distance >= (-10):  # Only display within the semi-circular area
            cursor_label.setText(f"Angle: {angle:.2f}° | Distance: {distance:.2f} mm\n X: {x:.2f} mm | Y: {y:.2f} mm")
            cursor_label.setPos(x, y)
            cursor_label.setColor('r')
        else:
            cursor_label.setText("")

proxy = pg.SignalProxy(plot.scene().sigMouseMoved, rateLimit=60, slot=mouse_moved)

# ----- Points and Age Logic -----
points = []
aged_points = []
scatter = pg.ScatterPlotItem(size=10, pen=None)
plot.addItem(scatter)
point_index = 0

def load_points_from_file(file_path):
    """Load angle,distance points from a text file."""
    global points
    try:
        with open(file_path, 'r') as file:
            lines = file.readlines()
            points = [tuple(map(float, line.strip().strip('.').split(','))) for line in lines if line.strip()]
    except Exception as e:
        print(f"Error reading file: {e}")

def test():
    print("here")

# def read_from_serial():
#     print("here")
#     global points
#     try:
#         data_buffer = ""  # Initialize buffer
#         while ser.in_waiting > 0:  
#             char = ser.read().decode('utf-8')  # Read one character at a time
#             print(char)
#             print("\n")
#             if char == ".":  # End of data point
#                 data_buffer = data_buffer.strip()  # Remove whitespace
#                 print(data_buffer)
#                 if "," in data_buffer:
#                     angle, distance = map(float, data_buffer.split(','))
#                     points.append((angle, distance))
#                 data_buffer = ""  # Reset buffer after processing
#             else:
#                 data_buffer += char  # Append character to buffer
#     except Exception as e:
#         print(f"Serial Read Error: {e}")

def read_from_serial():
    global points, is_reading, aged_points
    if is_reading:
        return  # Exit if the function is already running
    is_reading = True  # Set the flag to indicate the function is running
    try:
        data_buffer = ""  # Initialize buffer
        while ser.in_waiting > 0:
            char = ser.read().decode('utf-8')  # Read one character at a time
            if char == ".":  # End of data point
                data_buffer = data_buffer.strip()  # Remove whitespace
                if "," in data_buffer:
                    angle, distance = map(float, data_buffer.split(','))
                    points.append((angle, distance))
                    for p in aged_points:
                        p['age'] += 1
                data_buffer = ""  # Reset buffer after processing
            else:
                data_buffer += char  # Append character to buffer
    except Exception as e:
        print(f"Serial Read Error: {e}")
    finally:
        is_reading = False  # Reset the flag after finishing
        
    display_next_point()  # Update plot with new points


def update_plot():
    """Update points without clearing the radar background."""
    global aged_points
    
    if not aged_points:
        return

    remove = [p for p in aged_points if p['age'] >= maxAge]
    for p in remove:
        ClusterManager.removePoint((p['x'], p['y']))

    aged_points = [p for p in aged_points if p['age'] < maxAge]

    x_points = [p['x'] for p in aged_points]
    y_points = [p['y'] for p in aged_points]
    ages = [p['age'] for p in aged_points]

    min_opacity = 50  # Minimum opacity value
    
    colors = [
        (
            *ClusterManager.findPointColor((x, y)),  # Get RGB from the cluster color
            int(min_opacity + (255 - min_opacity) * (1 - age / maxAge))  # Calculate opacity based on age
        )
        for (x, y, age) in zip(x_points, y_points, ages)
    ]

    # min_red_intensity = 50  

    # colors = [
    #     (
    #         int(min_red_intensity + (255 - min_red_intensity) * (1 - age / maxAge)),
    #         0,
    #         0
    #     )
    #     for age in ages
    # ]

    spots = [{'pos': (x, y), 'brush': pg.mkBrush(color)} for x, y, color in zip(x_points, y_points, colors)]
    scatter.setData(spots=spots)


def display_next_point():
    """Display one point at a time while preserving previous points and radar background."""
    #print("here, display")

    global point_index, aged_points

    if(READ_FROM_FILE):
        for p in aged_points:
            p['age'] += 1

    #if not READ_FROM_FILE:
    #    read_from_serial()

    if point_index < len(points):
        angle, distance = points[point_index]
        x = distance * np.cos(np.radians(angle))
        y = distance * np.sin(np.radians(angle))
        aged_points.append({'x': x, 'y': y, 'age': 0})
        ClusterManager.processNewPoint((x,y))
        point_index += 1
        update_plot()
    else:
        if(READ_FROM_FILE):
            timer.stop()

# ----- Timer and Controls -----
if READ_FROM_FILE:
    # assuming this is where the data is read from
    client = mqtt.Client()
    # Connect to the MQTT broker
    client.connect(ip_address, broker_port)
    open_url_by_ip()

    load_points_from_file('sampleData3.txt')
    timer = QtCore.QTimer()
    timer.timeout.connect(display_next_point)
    timer.start(1000)

# # For serial, introduce a cooldown mechanism
# if not READ_FROM_FILE:
#     #print("here")
#     #timer = QtCore.QTimer()
#     #print("here")
#     #timer.timeout.connect(read_from_serial)  # Continuously read from serial and process data
#     QtCore.QTimer.singleShot(2, read_from_serial)
#     #timer.start(2)  # Read from serial port at intervals
#     #display_next_point()  # Immediately display the next point after reading from serial

if not READ_FROM_FILE:
    timer = QtCore.QTimer()
    timer.timeout.connect(read_from_serial)  # Continuously read from serial and process data
    timer.start(2)  # Read from serial port at intervals

# Add Escape key logic to close the window
def handle_key_press(event):
    if event.key() == QtCore.Qt.Key_Escape:
        win.close()

win.keyPressEvent = handle_key_press

if __name__ == '__main__':
    win.show()
    QtWidgets.QApplication.instance().exec_()
