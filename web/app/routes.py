from flask import render_template
from app import app
import boto3
import json

#Define what AWS services are used
iot = boto3.client('iot')
iotd = boto3.client('iot-data')

@app.route('/')
@app.route('/index')
def index():

    #Retrieve list of things. For now this prints to the console.
    #Later this can be used to automatically display all 'Things' on the web app
    print(iot.list_things())

    #List all shows associated with specific 'Thing'
    #This can be removed later if we decide to keep a single shadow with the same name
    print(iotd.list_named_shadows_for_thing(thingName="RemoteMonitor"))

    response = iotd.get_thing_shadow(
    thingName='RemoteMonitor',
    shadowName='properties'
    )
    
    shadow_string = response["payload"].read().decode('utf-8')
    shadow_json = json.loads(shadow_string) 


    battery_voltage = shadow_json["state"]["reported"]["battery_voltage"]
    mains_status = shadow_json["state"]["reported"]["mains_status"]

    print(battery_voltage)
    print(mains_status)

    return render_template('index.html', battery_voltage=battery_voltage, mains_status=mains_status)