from oauthlib.oauth2 import BackendApplicationClient
from requests_oauthlib import OAuth2Session

oauth_client = BackendApplicationClient(client_id="YMWO38PcFLvhwsTwXH5vluxifUs3FILY")
token_url = "https://api2.arduino.cc/iot/v1/clients/token"

oauth = OAuth2Session(client=oauth_client)
token = oauth.fetch_token(
    token_url=token_url,
    client_id="YMWO38PcFLvhwsTwXH5vluxifUs3FILY",
    client_secret="9tUHwu8EAWmGI3vROlWWJcMdotta94zxinneboUVsfgzS97Rkoy9v3d66L5Wppv0",
    include_client_id=True,
    audience="https://api2.arduino.cc/iot",
)

print(token.get("access_token"))

import iot_api_client as iot
from iot_api_client.rest import ApiException
from iot_api_client.configuration import Configuration

# configure and instance the API client
client_config = Configuration(host="https://api2.arduino.cc/iot")
client_config.access_token = token.get("access_token")
client = iot.ApiClient(client_config)

# as an example, interact with the devices API
devices_api = iot.DevicesV2Api(api_client=client)

"""try:
    resp = devices_api.devices_v2_list()
    print(resp)
except ApiException as e:
    print("Got an exception: {}".format(e))"""


id = "77cac144-c8de-4792-93ce-924c4443e8ef" # String | The id of the device (default to null)
#limit = 56 # Integer | The number of events to select (optional) (default to null)
#start = start_example # String | The time at which to start selecting events (optional) (default to null)

try: 
    # getEvents devices_v2
    api_response = devices_api.devices_v2_get_events_with_http_info(id)
    print(api_response)
except ApiException as e:
    print("Exception when calling DevicesV2Api->devicesV2GetEvents: %s\n" % e)

"""import time
import openapi_client
from openapi_client.rest import ApiException
from pprint import pprint

# Configure OAuth2 access token for authorization: oauth2
openapi_client.configuration.access_token = token.get("access_token")

# create an instance of the API class
api_instance = openapi_client.DevicesV2Api()
id = "YMWO38PcFLvhwsTwXH5vluxifUs3FILY" # String | The id of the device (default to null)
limit = 56 # Integer | The number of events to select (optional) (default to null)
start = start_example # String | The time at which to start selecting events (optional) (default to null)

try: 
    # getEvents devices_v2
    api_response = api_instance.devices_v2_get_events(id, limit=limit, start=start)
    pprint(api_response)
except ApiException as e:
    print("Exception when calling DevicesV2Api->devicesV2GetEvents: %s\n" % e)"""