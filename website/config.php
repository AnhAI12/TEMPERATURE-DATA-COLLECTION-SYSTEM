<?php
// Connect to database
$server = "localhost";
$user = "userpc"; 
$pass = "18";
$dbname = "lda_IOT";

$conn = mysqli_connect($server,$user,$pass,$dbname);

// Check connection
if($conn === false){
    die("ERROR: Could not connect. " . mysqli_connect_error());
}


?>