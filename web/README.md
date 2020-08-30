instructions for mac
change to web directory
1. Install 
1. to activate the virtual environment in the terminal = source venv/bin/activate
 - FLASK_AP=main.py
2. to run = flask run
3. to exit = control c

## How to Launch
1. Clone repository https://github.com/erik6K/remote-monitor from github and extract the *.zip* contents
2.  `cd remote-monitor/web`
3. Create virtual environmnent `python3 -m venv venv`
4. Enter virtual environment `source venv/bin/activate`
5. Install required packages `pip install -r requirements.txt`
6. `export AWS_DEFAULT_REGION=ap-southeast-2`
7. `export AWS_ACCESS_KEY_ID=<enter aws access key id>`
8. `export AWS_SECRET_ACCESS_KEY=<enter aws secret access key>`
6. `flask run`
7. Go to http://localhost:5000/