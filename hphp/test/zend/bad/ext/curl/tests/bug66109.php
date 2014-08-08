<?php
include 'server.inc';
$host = curl_cli_server_start();
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, "{$host}/get.php?test=method");
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'DELETE');
var_dump(curl_exec($ch));

curl_setopt($ch, CURLOPT_CUSTOMREQUEST, NULL);
var_dump(curl_exec($ch));

curl_close($ch);

?>
