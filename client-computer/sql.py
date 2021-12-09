#!/usr/bin/env python3
import paho.mqtt.client as mqtt
import requests
import time

def on_message(client, userdata, message):
    print("message received " ,str(message.payload.decode("utf-8")))
    print("message topic=",message.topic)

    if message.topic == "esp32/mode" :
        global mode
        mode = str(message.payload.decode("utf-8"))
    elif message.topic == "esp32/setpoint_temperature" :
        global setpoint_temperature
        setpoint_temperature = str(message.payload.decode("utf-8"))
    elif message.topic == "esp32/setpoint_humidity" :
        global setpoint_humidity
        setpoint_humidity = str(message.payload.decode("utf-8"))
    elif message.topic == "esp32/setpoint_led" :
        global setpoint_light
        setpoint_light = str(message.payload.decode("utf-8"))
    elif message.topic == "esp32/tempsens" :
        global temperature
        temperature = str(message.payload.decode("utf-8"))
    elif message.topic == "esp32/humsens" :
        global humidity
        humidity = str(message.payload.decode("utf-8"))
    elif message.topic == "esp32/ledsens" :
        global light 
        light = str(message.payload.decode("utf-8"))
    else :
        pass

server_ip = "34.101.153.146"
client = mqtt.Client("raspi3")
client.on_message = on_message
client.connect(server_ip)
client.loop_start()

mode = "1"
temperature = "0"
humidity = "0"
light = "0"
setpoint_temperature = "0"
setpoint_humidity = "0"
setpoint_light = "0"


#client.subscribe("esp32/heater",qos = 1)
#client.subscribe("esp32/cooler", qos = 1)
#client.subscribe("esp32/humidifier", qos = 1)
#client.subscribe("esp32/led", qos = 1)
#client.subscribe("esp32/fan", qos = 1)
#client.subscribe("esp32/pump", qos = 1)
client.subscribe("esp32/mode", qos = 1)

client.subscribe("esp32/setpoint_temperature", qos = 1)
client.subscribe("esp32/setpoint_humidity", qos = 1)
client.subscribe("esp32/setpoint_led", qos = 1)

client.subscribe("esp32/ledsens", qos = 1)
client.subscribe("esp32/tempsens", qos = 1)
client.subscribe("esp32/humsens", qos = 1)
#client.subscribe("esp32/watersens", qos = 1)
#client.subscribe("esp32/tempambientsens", qos = 1)

def sendData(): 
    filenames = {"mode_status" : mode, "sensor_temperature" : temperature, "sensor_humidity" : humidity,
                "sensor_light" : light, "setpoint_temperature" : setpoint_temperature, 
                "setpoint_humidity" : setpoint_humidity, "setpoint_light" : setpoint_light}
    try :
        flag_sql = requests.post("http://{}/SGC/RaspitoSQL.php".format(server_ip), data=filenames)
        return flag_sql
    except :
        pass
        

if __name__ == '__main__':
    while True :
        flag_sql = sendData()
        print("status to SQL : {}".format(flag_sql))
        time.sleep(120)
