<?php

<<__EntryPoint>>
function main_port_string() {
$mysqli = mysqli_init();
$mysqli->real_connect("localhost", "user", "password", "database", "3306");
}
