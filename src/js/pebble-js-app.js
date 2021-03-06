function compare(x, y) {
    if(x < y)
        return -1
    if(x > y)
        return 1
    return 0;
}

function getUniqueRoutes(response) {
	var routes = response.map(function (train) {
		    return {
		        name: train.ROUTE,
		        direction: train.DIRECTION
		    };
	});
	routes = routes.sort(function(x, y) {
		    var compareResult = compare(x.name, y.name);
		    if(compareResult == 0) {
		    	compareResult = compare(x.direction, y.direction);
		    }
		    return compareResult;
	});
	routes = routes.filter(function (val, idx, arry) {
		    return idx == 0 ||
		    	val.name != arry[idx - 1].name ||
		    	val.direction != arry[idx - 1].direction
	    	;
	});
	return routes;
}

function getTrains(response) {
	var routes = getUniqueRoutes(response);
	return routes
		.map(function(route) {
	    	var trainsForLine = response.filter(function(train) {
	       		return train.ROUTE == route.name
	            	&& train.DIRECTION == route.direction;
    		});

    		var routeName = route.name[0];
    		// Racist?
    		if(route.name == 'GOLD') {
    			routeName = 'Y';
    		}
		    return {
		        name: routeName,
		        direction: route.direction,
		        destination: trainsForLine[0].HEAD_SIGN.trim(),
		        waitTimes: trainsForLine.map(function(train) {
		            return train.WAITING_TIME;
		        })
		    };
		});
}


var url = 'http://developer.itsmarta.com/NextTrainService/RestServiceNextTrain.svc/GetNextTrain/';
/*
 * This is all a little yucky, but its to shrink down the size of the message
 * we send back to the pebble.
 */
function getTrainsForStation(stationName) {
    var slashIdx = stationName.indexOf('/');
    if(slashIdx > 0) {
    	stationName = stationName.substring(0, slashIdx);
    }
    var req = new XMLHttpRequest();
    req.open('GET', url + stationName, true);
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

Pebble.addEventListener("appmessage",
	function(e) {
		if(e.payload.fetchTrains) {
			console.log("Fetching trains for station " + e.payload.stationName);
			getTrainsForStation(e.payload.stationName);
		}
	}
);