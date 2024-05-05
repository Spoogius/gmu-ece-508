import paho.mqtt.client as mqtt

import numpy as np
import matplotlib.pyplot as plt
import json
import sys
import time 

def on_subscribe(client, userdata, mid, reason_code_list, properties):
    # Since we subscribed only for a single channel, reason_code_list contains
    # a single entry
    if reason_code_list[0].is_failure:
        print(f"Broker rejected you subscription: {reason_code_list[0]}")
    else:
        print(f"Broker granted the following QoS: {reason_code_list[0].value}")

def on_unsubscribe(client, userdata, mid, reason_code_list, properties):
    # Be careful, the reason_code_list is only present in MQTTv5.
    # In MQTTv3 it will always be empty
    if len(reason_code_list) == 0 or not reason_code_list[0].is_failure:
        print("unsubscribe succeeded (if SUBACK is received in MQTTv3 it success)")
    else:
        print(f"Broker replied with failure: {reason_code_list[0]}")
    client.disconnect()

def on_message(client, userdata, message):
   
    msg = json.loads( message.payload )
    print(f"Message from topic {message.topic}")
    print( msg )
    
    if( message.topic == config['topic_adc'] ):
      bin_freq = float(msg["peak_freq"])
      bin_mag = 20*np.log10(float(msg["peak_mag"])/128)
      print(f"dB Mag: {bin_mag} @ {bin_freq}")
      if( np.abs( bin_freq-config["fan_freq_bin"]) < abs(config["fan_freq_bin"]*0.05)): #Within a 5% tollerance     
        if( bin_mag > config["fan_mag_bin"] ): # Above threshold
          if( curr_state == True ):
            print(f"Error: Fan should be on, but isn't detected")

    elif( message.topic == config['topic_dht'] ):
      # Moving average of 5 measurments
      dht_arr[dht_idx % len(dht_arr)] = msg['humidity']
      dht_avg = np.mean( dht_arr )
      print(f"Updated Humidity Avg {dht_avg} RH%")
      if( dht_avg > float(config["humidity_threshold"]) ):
        if( curr_hw_state == False ):
          set_hw_state( 1 ) # Turn on Fan 
      else: # Below Threshold
        if( curr_hw_state == True ):
          set_hw_state( 0 ) # Turn off Fan

def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code.is_failure:
        print(f"Failed to connect: {reason_code}. loop_forever() will retry connection")
    else:
        # we should always subscribe from on_connect callback to be sure
        # our subscribed is persisted across reconnections.
        client.subscribe(config["topic_adc"])
        client.subscribe(config["topic_dht"])


def on_publish(client, userdata, mid, reason_code, properties):
  print(f"Message {mid} published")

def set_hw_state( state ):
  out_msg = {};
  out_msg["timestamp"] = time.time()
  out_msg["source"] = "server"
  out_msg["fan_control"] = int(state)

  out_msg_str = json.dumps(out_msg)
  print(f"Publishing: {out_msg_str}")
  out_msg_info = mqttc.publish( config['topic_ctrl'], out_msg_str, qos=1)

# Load Config
with open(sys.argv[1], 'r' ) as fin:
  config = json.load( fin )
  print( config )

curr_hw_state = False

# DHT Average
dht_arr = [0,0,0,0,0,0,0,0,0,0,0,0]
dht_idx = 0



unacked_publish = set()
mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqttc.on_connect = on_connect
mqttc.on_message = on_message
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe
mqttc.on_publish = on_publish
mqttc.user_data_set(unacked_publish)
mqttc.connect(config["broker"], config["port"])


mqttc.loop_forever()
