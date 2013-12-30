Pebble.addEventListener("ready", function(e) {
    consoloe.log("JS is ready...");
});

Pebble.addEventListener("appmessage", function(e) {
	console.log("Received message: " + e.payload);
    if(e.payload.stationName) {
        getTrainsForStation(e.payload.stationName);
    }
});

function getTrainsForStation(stationName) {
    console.log("Getting data for " + stationName);
    var req = new XMLHttpRequest();
    req.open('GET', 'http://developer.itsmarta.com/NextTrainService/RestServiceNextTrain.svc/GetNextTrain/' + stationName, true);
    req.onload = function(e) {
        if (req.readyState == 4) {
            if(req.status == 200) {
                var response = JSON.parse(req.responseText);
                var i = 0;
                var messages = [];
                while(i < response.length && i < 4) {
                    var train = response[i];
                    var destination = train.HEAD_SIGN.trim();
                    var direction = train.DIRECTION;
                    var line = train.ROUTE;
                    var waitTime = train.WAITING_TIME;
                    messages.push(destination + " in " + waitTime + " min" );
                    i += 1;
                }

                Pebble.sendAppMessage(
                    {
                        incomingTrain1: messages[0],
                        incomingTrain2: messages[1],
                        incomingTrain3: messages[2],
                        incomingTrain4: messages[3]
                    }
                );
            } else { console.log("Error"); }
        }
    }
    req.send(null);
}