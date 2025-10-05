import paho.mqtt.client as mqtt
import time
import os
import sys

ip = '10.26.46.144' #this is the Pi's IP

def on_connect(client, userdata, flags, rc):
	global flag_connected
	flag_connected = 1
	client_subscriptions(client)
	print("connected to MQTT server")

def on_disconnect(client, userdata, rc):
	global flag_connected
	flag_connected = 0
	print("disconnected to MQTT server")

def callback_esp32_sensor1(client, userdata, msg):
    edit_html('index.html', str(msg.payload.decode('utf-8')))

def callback_esp32_sensor2(client, userdata, msg):
	print('ESP sensor2 data: ', str(msg.payload.decode('utf-8')))
246
def callback_rpi_broadcast(client, userdata, msg):
	print ('RPI broadcast message: ', str(msg.payload.decode('utf-8')))

def client_subscriptions(client):
	client.subscribe("esp32/#")
	client.subscribe("rpi/broadcast")

def edit_html(filename, new_data, directory="/var/www/html"):
    file_path = os.path.join(directory, filename)

    if not os.path.exists(file_path):
        print(f"Error: {file_path} does not exist")
        return
    try:
        with open(file_path, 'w', encoding='utf-8') as file:
            file.write(new_data)

        print(f"File {filename} successfully updated with new data")

    except Exception as e:
        print(f"Error editing file: {e}")


client = mqtt.Client("rpi_client1") #unique name
flag_connected = 0

client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.message_callback_add('esp32/sensor1', callback_esp32_sensor1)
client.message_callback_add('esp32/sensor2', callback_esp32_sensor2)
client.message_callback_add('rpi/broadcast', callback_rpi_broadcast)
client.connect(ip,1883) #pi IP
print('Connected :)')


client.loop_start()
client_subscriptions(client)
print(".....client setup complete.....")

while True:
	time.sleep(4)
	if(flag_connected != 1):
		print("trying to connect to MQTT server")
