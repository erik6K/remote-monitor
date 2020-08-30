from oauthlib.oauth2 import BackendApplicationClient
from requests_oauthlib import OAuth2Session

oauth_client = BackendApplicationClient(client_id="YMWO38PcFLvhwsTwXH5vluxifUs3FILY")
token_url = "https://api2.arduino.cc/iot/v1/clients/token"

oauth = OAuth2Session(client=oauth_client)
token = oauth.fetch_token(
    token_url=token_url,
    client_id="YMWO38PcFLvhwsTwXH5vluxifUs3FILY",
    client_secret="l4e8tprzXt1NTIaUPvg8Juo9FSydI5IGZNSM0rmRG9EbMnCWJHj7HIb18kSo5b44",
    include_client_id=True,
    audience="https://api2.arduino.cc/iot",
)

import iot_api_client as iot
from iot_api_client.rest import ApiException
from iot_api_client.configuration import Configuration

# configure and instance the API client
client_config = Configuration(host="https://api2.arduino.cc/iot")
client_config.access_token = token.get("access_token")
client = iot.ApiClient(client_config)

# as an example, interact with the devices API
devices_api = iot.DevicesV2Api(client)
things_api = iot.ThingsV2Api()
"""
try:
    resp = devices_api.devices_v2_list()
    print(resp)
except ApiException as e:
    print("Got an exception: {}".format(e))
    
"""



id = "c0945b18-f453-4963-8043-3f053b3fbe8d" # String | The id of the device (default to null)
#limit = 56 # Integer | The number of events to select (optional) (default to null)
#start = start_example # String | The time at which to start selecting events (optional) (default to null)
try: 
    # getEvents devices_v2
    api_response = things_api.things_v2_show(id)
    print(api_response)
except ApiException as e:
    print("Got an exception: {}".format(e))
