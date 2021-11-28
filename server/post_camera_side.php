<?php

/* [ Source Code ]
Date              : November 27, 2021
Dependencies      : -

Description: Receive images data from Raspberry Pi
*/

$max_pictures_saved = 10; //set maximum number of pictures saved at the same time
$upload_dir = '/opt/lampp/htdocs/SGC/camera_upload/camera_side';  //directory to save pictures
$name = basename($_FILES["side_camera"]["name"]); //filename
$target_file = "$upload_dir/$name";

$servername = "localhost";
$dbname = "sgc";
$username = "root";
$password = "";

// Create connection to database
$conn = new mysqli($servername, $username, $password, $dbname);

//Move received file to directory 
if (!move_uploaded_file($_FILES["side_camera"]["tmp_name"], $target_file)){
    echo "error";
}
else {

if ($conn->connect_error) {
 die("Connection failed: " . $conn->connect_error);
}

//take filename of picture from databas and delete the file
$sqlAct = "SELECT id, top_camera_filename, back_camera_filename, side_camera_filename, reading_time FROM camera ORDER BY id DESC";
if ($result = $conn->query($sqlAct)) {
if($result->num_rows > $max_pictures_saved-1){
$i = 0;
while($i < $max_pictures_saved){
	$row = $result->fetch_assoc();
	$i = $i + 1;
	}
$deleted = $row["side_camera_filename"];	
if(file_exists($upload_dir . "/" . $deleted)){
	unlink($upload_dir . "/" . $deleted);
	}
}
$result->free();
}
    echo "success";
}

$conn->close();

?>
