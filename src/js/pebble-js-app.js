function compare(x, y) {
    if(x < y)
        return -1
    if(x > y)
        return 1
    return 0;
}

function getUniqueRoutes(response) {
	return response
		.map(function (train) {
		    return {
		        name: train.ROUTE,
		        direction: train.DIRECTION
		    };
		}).sort(function(x, y) {
		    return compare(x.name, y.name);
		}).filter(function (val, idx, arry) {
		    return idx == 0 || val.direction != arry[idx - 1].direction;
		});
}

function getTrains(response) {
	return getUniqueRoutes(response)
		.map(function(route) {
	    	var trainsForLine = response.filter(function(train) {
	       		return train.ROUTE == route.name
	            	&& train.DIRECTION == route.direction;
    		});
	    
		    return {
		        name: route.name[0],
		        direction: route.direction,
		        destination: trainsForLine[0].HEAD_SIGN.trim(),
		        waitTimes: trainsForLine.map(function(train) {
		            return train.WAITING_TIME;
		        })
		    };
		});
}

/* 
 * This is all a little yucky, but its to shrink down the size of the message
 * we send back to the pebble.
 */
function getTrainsForStation(stationName) {
    console.log("Getting data for " + stationName);
    var req = new XMLHttpRequest();
    req.open('GET', 'http://developer.itsmarta.com/NextTrainService/RestServiceNextTrain.svc/GetNextTrain/' + stationName, true);
    req.onload = function(e) {
        if (req.readyState == 4) {
            if(req.status == 200) {
                var response = JSON.parse(req.responseText);

                var trains = getTrains(response)
                	.map(function(rnt) {
				    	var timeStr = rnt.waitTimes.join(',');
				    	return [rnt.name, rnt.direction, timeStr].join(';');
					}).join('|');
				console.log("Message to send: " + trains);
				Pebble.sendAppMessage({
					"trainInfo": trains
				});
            } else { console.log("Error"); }
        }
    }
    req.send(null);
}

Pebble.addEventListener("ready",
    function(e) {
        console.log("Hello world! - Sent from your javascript application.");
    }
);

Pebble.addEventListener("appmessage",
	function(e) {
		console.log("Recieved a message");
		if(e.payload.fetchTrains) {
			console.log("Fetching trains for station " + e.payload.stationName);
			getTrainsForStation(e.payload.stationName);
		}
	}
);