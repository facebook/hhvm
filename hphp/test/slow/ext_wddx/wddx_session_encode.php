<?php
ini_set("session.serialize_handler","wddx");
session_start();
$_SESSION['login_ok'] = true;
$_SESSION['name'] = 'somename';
$_SESSION['integer'] = 34;
var_dump(session_encode());
