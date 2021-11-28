<?php

/* [ Source Code ]
Date              : November 27, 2021
Dependencies      : -

Description: Receive data from Client (Raspberry Pi) and insert data into MySQL Database
*/

$servername = "localhost";

// REPLACE with your Database name
$dbname = "sgc";
// REPLACE with Database user
$username = "root";
// REPLACE with Database user password
$password = "";

//When received HTTP POST request from client
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    //$api_key = test_input($_POST["api_key"]);
    //if($api_key == $api_key_value) {
    $filename =  $_POST['top_camera_filename'];
    $filename2 = $_POST['back_camera_filename'];
    $filename3 = $_POST['side_camera_filename'];
        
    // Create connection to database
    $conn = new mysqli($servername, $username, $password, $dbname);
    // Check connection
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    } 
    
    //Insert data into database
    $sql = "INSERT INTO camera (top_camera_filename, back_camera_filename, side_camera_filename)
    VALUES ('" . $filename . "', '" . $filename2 . "', '" . $filename3 . "')";
        
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
