import requests
from datetime import datetime, timedelta
# import json

# This API name should not be versioned - for obvious reasons - too late
dataRaw = requests.get(
    'https://api2-mtc.gazzetta.it/api/v1/sports/calendar?sportId=1&competitionId=21')

# data = json.load(dataRaw.text)
data = dataRaw.json()

# print(data['data']['games'][0]['matches'][0])

# Goals
# - Find first match datetime
# - Find last match datetime
# - Save to file both info

# Sort 'games' array since we don't know if ordered
def getDatetimeForSort(el):
    # Formatting date string to be parsed by pyhon
    i = el['date'].index('+')
    dStrip = el['date'][0:i]
    return datetime.strptime(dStrip, '%Y-%m-%dT%H:%M:%S')


# Unite all sub arrays into one unique array
allMataches = []
for day in data['data']['games']:
    for match in day['matches']:
        allMataches.append(match)

# Sort
allMataches.sort(key=getDatetimeForSort)

with open("dueFormazza.txt", 'w') as f:
    dNow = datetime.now()
    # Get datetime of first match
    dDate = getDatetimeForSort(allMataches[0])

    # Get remaining time, including 15 minutes of advance
    dDiff = dDate - dNow - timedelta(minutes=15)
    hoursForma, rem = divmod(dDiff.seconds, 3600)
    minutesForma, seconds = divmod(rem, 60)

    # Format fanta due date
    strFanta = 'Fanta+'
    if dDiff.days > 1:
        strFanta += str(dDiff.days) + ' days '
    elif dDiff.days == 1:
        strFanta += '1 day'
    
    strFanta += '{}:{}'.format(hoursForma, minutesForma)

    # Get current date time formatted
    strNow = 'Time+' + dNow.strftime("%d/%m/%Y %H:%M")

    # Write to file
    f.write(strNow + '+' + strFanta)
