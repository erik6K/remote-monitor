from flask import render_template, flash
from app import app
import requests
import json

@app.route('/')
@app.route('/index')
def index():
    request = requests.get('https://remote-monitor.azureiotcentral.com/api/preview/devices', headers={'Authorization': app.config['AZURE_TOKEN']})
    if not request.ok:
        flash('There was an error retirving the device list. Please contact the developers.')
        return render_template('error.html')
    payload = request.json()
    devices = payload['value']
    return render_template('index.html', devices=devices)

@app.route('/telemetry/<device_id>')
def telemetry(device_id):
    url = 'https://remote-monitor.azureiotcentral.com/api/preview/devices/'+device_id+'/components/'+app.config['COMPONENT']+'/telemetry/'
    mains_req = requests.get(url+'MainsStatus', headers={'Authorization': app.config['AZURE_TOKEN']})
    mains = mains_req.json()
    battery_req = requests.get(url+'BatteryVoltage', headers={'Authorization': app.config['AZURE_TOKEN']})
    battery = battery_req.json()
    telemetry = {
        'mains': mains,
        'battery': battery
    }
    if not mains_req.ok or not battery_req.ok:
        return "404"
    return telemetry

