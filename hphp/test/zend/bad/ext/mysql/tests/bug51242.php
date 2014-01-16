<?php
require_once('connect.inc');

if ($host == 'localhost') {
    $host = '127.0.0.1';
}

if ($link = my_mysql_connect($host, $user, $passwd, $db, null, $socket)) {
    var_dump($link);
} else {
    printf("[001] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
            $host, $user, $db, null, $socket);
}
?>