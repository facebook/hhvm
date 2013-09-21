<?php

$socket = socket_create_listen(31339);
var_dump(socket_set_block($socket));
socket_close($socket);

$socket2 = socket_create_listen(31340);
socket_close($socket2);
var_dump(socket_set_block($socket2));

?>