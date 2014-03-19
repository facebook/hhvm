<?php
    $rand = rand(1,999);
    // wrong parameter count
    $s_w = socket_write();
    $s_c = socket_create_listen(31330+$rand);
    $s_w = socket_write($s_c);
    $s_w = socket_write($s_c, "foo");
    socket_close($s_c);
?>