<?php

<<__EntryPoint>>
function main_session_php_serialize() {
ini_set("session.serialize_handler", "php_serialize");
session_start();
$_SESSION["data"] = "sjc";
var_dump(session_encode());
}
