from flask import render_template
from app import app
import requests
import json

@app.route('/')
@app.route('/index')
def index():
    payload_raw = requests.get('https://remote-monitor.azureiotcentral.com/api/preview/devices', headers={'Authorization': app.config['AZURE_TOKEN']}).content
    payload = json.loads(payload_raw.decode('utf-8'))
    devices = payload['value']

    for device in devices:
        print(device)

    return render_template('index.html')

@app.route('/telemetry')
def telemetry():
    return devices

