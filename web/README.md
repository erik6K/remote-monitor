## How to Launch
1. Clone repository https://github.com/erik6K/remote-monitor from github and extract the *.zip* contents
2.  `cd remote-monitor/web`
3. Create virtual environmnent `python3 -m venv venv`
4. Enter virtual environment `source venv/bin/activate`
5. Install required packages `pip install -r requirements.txt`
6. `export AWS_DEFAULT_REGION=ap-southeast-2`
7. `export AWS_ACCESS_KEY_ID=<enter aws access key id>`
8. `export AWS_SECRET_ACCESS_KEY=<enter aws secret access key>`
9. `flask run`
10. Go to http://localhost:5000/
11. To exit use Ctrl+C or control+C