# Ultrasonic Radar Display

school project for detecting objects using ultrasonic sensors. ESP32 sends data to a raspberry pi over wifi and python displays it like a radar screen.

## what it does

uses 6 ultrasonic transmitters to scan 180 degrees and detect objects up to 1 meter away. the display shows points that fade out over time and groups nearby detections into colored clusters.

## setup

### you need:
- ESP32 board
- 6x 40kHz ultrasonic transmitters (pins 35,36,37,38,39,40)
- 1x 40kHz receiver (pin 7)
- raspberry pi with MQTT broker running

### install python stuff:
```bash
pip install pyqtgraph PyQt5 numpy pyserial paho-mqtt
```

### configure the display (radar_display.py):
```python
READ_TYPE = 2  # 0 = test file, 1 = serial, 2 = wifi

# if using serial (READ_TYPE = 1):
SERIAL_PORT = "COM4"  # change to your port
BAUD_RATE = 115200

# if using wifi (READ_TYPE = 2):
ip_address = "xx.xx.xxx.xxx"  # your raspberry pi IP
```

### configure ESP32 code:
```cpp
const char *ssid = "sdmay25-36";
const char *password = "password";
const char* mqtt_server = "10.xxx.xxx.xxx";  // raspberry pi IP
```

## running it

1. make sure raspberry pi MQTT broker is running
2. upload code to ESP32
3. run the display:
```bash
python radar_display.py
```

press ESC to exit fullscreen. hover mouse to see angle/distance.

## how it works

ESP32 uses phased array beam steering to scan different angles. calculates time delays between transmitters to point the beam. measures time-of-flight for distance. sends data as `angle,distance.` over MQTT.

python script:
- reads incoming data
- groups nearby points into clusters (different colors)
- fades out old points over time
- displays everything on a radar-style plot

## files

- `radar_display.py` - visualization code
- ESP32 code - microcontroller firmware
- `sampleData3.txt` - sample data for testing without hardware

## notes

max range is 1000mm. scans from -60 to +60 degrees. receiver pin 7 reads analog values, threshold is 800 (~0.76v). points stick around for 10 frames before fading completely.

made for sdmay25-36 at iowa state