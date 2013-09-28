<?php

$r = socket_create(AF_INET, SOCK_RAW, 0);
var_dump(socket_last_error());
