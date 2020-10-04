## How to Launch
1. Clone repository https://github.com/erik6K/remote-monitor from github and extract the *.zip* contents
2.  `cd remote-monitor/web`
3. Create virtual environmnent `python3 -m venv venv`
4. Enter virtual environment `source venv/bin/activate` (or for Windows `venv\Scripts\activate`)
NOTE: If you recieve a 'running scripts is disabled error' on Windows, try running `Set-ExecutionPolicy RemoteSigned` first. 
5. Install required packages `pip install -r requirements.txt`
6. `export AZURE_TOKEN="<enter token>"`(for mac/linux)
7. `export AZURE_COMPONENT="<enter component id>"`(for mac/linux)
8. `setx AZURE_TOKEN "<enter token>"` (for windows)
9. `setx AZURE_COMPONENT "<enter component id>"` (for windows)
10. `flask run`
11. Go to http://localhost:5000/
12. To exit use Ctrl+C or control+C