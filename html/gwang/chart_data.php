<?php
		//address of the server where db is installed
		$servername = "localhost";
		//username to connect to the db
		//the default value is root
		$username = "root";
		//password to connect to the db
		//this is the value you would have specified during installation of WAMP stack
		$password = "root";
		//name of the db under which the table is created
		$dbName = "capstone";
		//establishing the connection to the db.
		$conn = new mysqli($servername, $username, $password, $dbName);
		//checking if there were any error during the last connection attempt
		if ($conn->connect_error) {
		  die("Connection failed: " . $conn->connect_error);
		}
		//the SQL query to be executed
		$query = "SELECT Building_Id, Elevator_id, Current_Floor FROM getCurr";
		//storing the result of the executed query
		$result = $conn->query($query);
		//initialize the array to store the processed data
		$jsonArray = array();
		//check if there is any data returned by the SQL Query
		if ($result->num_rows > 0) {
		  //Converting the results into an associative array
		  while($row = $result->fetch_assoc()) {
		    $jsonArrayItem = array();
		    $jsonArrayItem['building'] = $row['Building_Id'];
		    $jsonArrayItem['elevator'] = $row['Elevator_id'];
		    $jsonArrayItem['floor'] = $row['Current_Floor'];
		    //append the above created object into the main array.
		    array_push($jsonArray, $jsonArrayItem);
		  }
		}
		//Closing the connection to DB
		$conn->close();
		//set the response content type as JSON
		header('Content-type: application/json');
		//output the return value of json encode using the echo function. 
		echo json_encode($jsonArray);
	?>
var apiChart = new FusionCharts({
	  type: 'column2d',
	  renderAt: 'api-chart-container',
	  width: '550',
	  height: '350',
	  dataFormat: 'json',
	  dataSource: {
	    "chart": chartProperties,
	    "data": chartData
	  }
	});
	$(function(){
	  $("#background-btn").click(function(){
	    modifyBackground();
	  });
	 
	  $("#canvas-btn").click(function(){
	    modifyCanvas();
	  });
	 
	  $("#dataplot-btn").click(function(){
	    modifyDataplot();
	  });
	 
	  apiChart.render();
	});
	 
	function modifyBackground(){
	  //to be implemented
	}
	 
	function modifyCanvas(){
	  //to be implemented
	}
	 
	function modifyDataplot(){
	  //to be implemented
	}
