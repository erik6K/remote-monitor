import boto3
import json

iot = boto3.client('iot')
iotd = boto3.client('iot-data')


print(iot.list_things())


print(iotd.list_named_shadows_for_thing(thingName="RemoteMonitor"))

response = iotd.get_thing_shadow(
    thingName='RemoteMonitor',
    shadowName='properties'
)

shadow_string = response["payload"].read().decode('utf-8')
shadow_json = json.loads(shadow_string) 
print(shadow_json["state"]["reported"]["battery_voltage"])
print(shadow_json["state"]["reported"]["mains_status"])

while(True) {
    wait()
}
