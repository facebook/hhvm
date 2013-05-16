<?php
    $rand = rand(1,999);
    // wrong parameter count
    $s_c = socket_read();
    $s_c = socket_read(14);
    $s_c_l = socket_create_listen(31330+$rand);
    $s_c = socket_read($s_c_l, 25);
    socket_close($s_c_l);
?>