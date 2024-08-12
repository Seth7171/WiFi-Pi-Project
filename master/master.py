import paho.mqtt.client as mqtt
import os

# MQTT Broker details
broker = "10.0.0.23"
port = 1883

# Topic names
temperature_topic = "esp32/temperature"
humidity_topic = "esp32/humidity"
light_topic = "esp32/light"
command_topic = "esp32/command"

commands = ["TOGGLE_RED_LED", "TOGGLE_BLUE_LED", "TOGGLE_WS2812B_WHITE", "TOGGLE_BUZZER"]       

# Function to clear the screen
def clear_screen():
    os.system('cls' if os.name == 'nt' else 'clear')

# MQTT callback functions
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe([(temperature_topic, 0), (humidity_topic, 0), (light_topic, 0)])

temp = "N/A"
humidity = "N/A"
light = "N/A"

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    global temp, humidity, light
    clear_screen()
    print("Sensor Data:")
    if msg.topic == temperature_topic:
        temp = msg.payload.decode()
    elif msg.topic == humidity_topic:
        humidity = msg.payload.decode()
    elif msg.topic == light_topic:
        light = msg.payload.decode()
    print(f"Temperature: {temp} °C")
    print(f"Humidity: {humidity} %")
    print(f"Light Intensity: {light}")
    print("\nCommands:")
    print("1 - Toggle the Red LED")
    print("2 - Toggle the Blue LED")
    print("3 - Toggle the WS2812B LEDs in White")
    print("4 - Toggle the Buzzer")
    print("Enter a command: ")

# Publish command to toggle an LED or buzzer
def send_command(command):
    client.publish(command_topic, commands[command-1])

# MQTT client setup
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to MQTT Broker
client.connect(broker, port, 60)

# Start the loop to process MQTT messages
client.loop_start()

while True:
    command = int(input(""))
    send_command(command)