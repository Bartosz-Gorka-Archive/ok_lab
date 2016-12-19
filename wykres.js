google.charts.load("current", {packages:["timeline"]});

var chart;
function drawChart(container, table) {
	chart = new google.visualization.Timeline(container);
	
	var dataTable = google.visualization.arrayToDataTable([
		['Role', 'name',{role: 'style'}, 'start', 'end']
	].concat(table));
	
	var options = {
		height:175,
		stroke: '#F00',
		strokeWidth: 1,
		timeline: { groupByRowLabel: true,colorByRowLabel: false }
	};
	chart.draw(dataTable, options);
}

function createDiv() {
	return document.querySelector(".template .result").cloneNode(true);
}

var resultsDiv = document.getElementById('results');

function process(e, fileName){
	var text = e.target.result;
	
	var div = createDiv();
	
	//console.log(text);
	var lines = text.split("\n")
	var graphTitle = lines[0].slice(5, -5);
	var goalValue = parseInt(lines[1].split(",")[0]);
	var maintM1count = parseInt(lines[4].split(",")[0]);
	var maintM1duration = parseInt(lines[4].split(",")[1]);
	var maintM2count = parseInt(lines[5].split(",")[0]);
	var maintM2duration = parseInt(lines[5].split(",")[1]);
	var idleM1count = parseInt(lines[6].split(",")[0]);
	var idleM1duration = parseInt(lines[6].split(",")[1]);
	var idleM2count = parseInt(lines[7].split(",")[0]);
	var idleM2duration = parseInt(lines[7].split(",")[1]);
	
	div.querySelector(".nazwaPliku").innerHTML = fileName;
	div.querySelector(".wartoscFCelu").innerHTML = goalValue;
	div.querySelector(".tytulGrafu").innerHTML = graphTitle;
	div.querySelector(".lPrzerwM1").innerHTML = maintM1count;
	div.querySelector(".cPrzerwM1").innerHTML = maintM1duration;
	div.querySelector(".lPrzerwM2").innerHTML = maintM2count;
	div.querySelector(".cPrzerwM2").innerHTML = maintM2duration;
	div.querySelector(".lIdleM1").innerHTML = idleM1count;
	div.querySelector(".cIdleM1").innerHTML = idleM1duration;
	div.querySelector(".lIdleM2").innerHTML = idleM2count;
	div.querySelector(".cIdleM2").innerHTML = idleM2duration;
	
	var taskColor2 = '#81F495';
	var taskColor1 = '#96F550';
	//var taskColor = 'stroke="black" stroke-width=15';
	var maintColor1 = '#006DAA';
	var maintColor2 = '#1B98E0';
	var idleColor1 = '#EDD3C4';
	var idleColor2 = '#C8ADC0';
	
	var tableToGraph = [];
	
	for(var k=2;k<4;k++){
		var M = lines[k].slice(3).replace(/ /g,"").split(";");
		var Mname = "";
		var o=0,d=0,m=0;
		if(k==2) Mname = "M1";
		else Mname = "M2";
		for(var i=0;i<M.length;i++){
			var task = M[i];
			if(task=="") continue;
			var tempTaskTab = task.split(",");
			var opName = tempTaskTab[0];
			var opColor
			if( opName[0]=="o") {
				if(o%2==0) opColor = taskColor1;
				else opColor = taskColor2;
				o++;
			}
			else if( opName[0]=="m"){
				if(m%2==0) opColor = maintColor1;
				else opColor = maintColor2;
				m++;
			}
			else if( opName[0]=="i"){
				if(d%2==0) opColor = idleColor1;
				else opColor = idleColor2;
				d++;
			}
			else opColor = '#ff0000';
			//opColor = opColor+',\'stroke-width\'="0"';
			
			var opStart = parseInt(tempTaskTab[1]);
			var opEnd = parseInt(tempTaskTab[2])+ opStart;
			tableToGraph.push([Mname,opName,opColor,opStart,opEnd]);
		}
	}
	resultsDiv.appendChild(div);
	drawChart(div.querySelector('.poleWykresu'), tableToGraph);
	
}

function makeProcess(fileName){
	return function(e) {
		process(e, fileName);
	}
};

document.getElementById('upload').addEventListener('change', function(e) {
	resultsDiv.innerHTML = '';
	for(var i=0;i<this.files.length;i++) {
		var file=this.files[i];
		var reader = new FileReader();
		reader.onloadend = makeProcess(file.name);
		reader.readAsText(file);
	}
});

document.getElementById('buttonMin').addEventListener('click', function(e) {
	var minimum = Math.min.apply(null, [].slice.call(document.querySelectorAll('#results .result .wartoscFCelu')).map(x=>parseInt(x.innerHTML)));
	//alert("Minimum ze Wszystkiego to: \n"+minimum);
	document.getElementById('wartoscMin').innerHTML = minimum;
});
