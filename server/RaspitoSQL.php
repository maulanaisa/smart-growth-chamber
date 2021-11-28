<?php

/* [ Source Code ]
Date              : November 27, 2021
Dependencies      : -

Description: Receive data from Client (Raspberry Pi) and insert it into MySQL Database
*/

$servername = "localhost";

// REPLACE with your Database name
$dbname = "sgc";
// REPLACE with Database user
$username = "root";
// REPLACE with Database user password
$password = "";

//Set variables to store data
$mode_status = $sensor_temperature = $sensor_humidity = $sensor_light = $setpoint_temperature = $setpoint_humidity = $setpoint_light =  "";

//When received HTTP POST request from client
if ($_SERVER["REQUEST_METHOD"] == "POST") {
	//Store data into variables
	$mode_status = $_POST["mode_status"];
        $sensor_temperature = $_POST["sensor_temperature"];
        $sensor_humidity = $_POST["sensor_humidity"];
        $sensor_light = $_POST["sensor_light"];
	$setpoint_temperature =  $_POST["setpoint_temperature"];
        $setpoint_humidity = $_POST["setpoint_humidity"];
        $setpoint_light = $_POST["setpoint_light"];
	
        if($mode_status == "0"){
            $mode_status = "manual";
        } else {
            $mode_status = "auto";
        }

        // Create connection to database
        $conn = new mysqli($servername, $username, $password, $dbname);
        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        } 
        
	//Insert data into database
        $sql = "INSERT INTO sensor (mode, sensor_temperature, sensor_humidity, sensor_light, 			setpoint_temperature, setpoint_humidity, setpoint_light)
        VALUES ('" . $mode_status . "', '" . $sensor_temperature . "', '" . $sensor_humidity . "', '" . 	$sensor_light . "', '" . $setpoint_temperature . "', '" . $setpoint_humidity . "', '" . $setpoint_light . "')";
        
        if ($conn->query($sql) === TRUE) {
            echo "New record created successfully";
        } 
        else {
            echo "Error: " . $sql . "<br>" . $conn->error;
        }
    
        $conn->close();

}
else {
    echo "No data posted with HTTP POST.";
}
?>


