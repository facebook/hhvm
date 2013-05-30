<?php

$session_handler = new SessionHandler();
session_set_save_handler(
    array($session_handler, 'open'),
    array($session_handler, 'close'),
    array($session_handler, 'read'),
    array($session_handler, 'write'),
    array($session_handler, 'destroy'),
    array($session_handler, 'gc')
);

$_SESSION['a'] = 'A';
var_dump($_SESSION);
