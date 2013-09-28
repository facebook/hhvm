<?php
$sockets = null;
$write   = null;
$except  = null;
$time    = 0;
var_dump(socket_select($sockets, $write, $except, $time));
socket_select($sockets, $write, $except);