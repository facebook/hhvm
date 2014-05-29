<?php
error_reporting(E_ALL & ~E_WARNING);
session_start();
session_destroy();

ini_set("session.hash_bits_per_character", 6);
ini_set("session.hash_function", 1);
session_start();
var_dump(strlen(session_id()));
session_destroy();

ini_set("session.hash_bits_per_character", 5);
ini_set("session.hash_function", 1);
session_start();
var_dump(strlen(session_id()));
session_destroy();

ini_set("session.hash_bits_per_character", 4);
ini_set("session.hash_function", 1);
session_start();
var_dump(strlen(session_id()));
session_destroy();
