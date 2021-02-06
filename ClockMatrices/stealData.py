import requests
from datetime import datetime
# import json

# This API name should not be versioned - for obvious reasons
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


def getTimestampForSort(el):
    i = el['date'].index('+')
    dStrip = el['date'][0:i]
    dtObj = datetime.strptime(dStrip, '%Y-%m-%dT%H:%M:%S')
    return int(datetime.timestamp(dtObj))


# Unite all sub arrays into one unique array
allMataches = []
for day in data['data']['games']:
    for match in day['matches']:
        allMataches.append(match)

allMataches.sort(key=getTimestampForSort)

with open("dueFormazza.txt", 'w') as f:
    dNow = datetime.now()

    i = allMataches[0]['date'].index('+')
    dStrip = allMataches[0]['date'][0:i]
    dDate = datetime.strptime(dStrip, '%Y-%m-%dT%H:%M:%S')

    dDiff = dDate - dNow
    hours, rem = divmod(dDiff.seconds, 3600)
    minutes, seconds = divmod(rem, 60)

    # str(getTimestampForSort(allMataches[0]))
    f.write('Fanta+{} giorni {} ore {} minuti'.format(dDiff.days,
                                                   hours, minutes))
