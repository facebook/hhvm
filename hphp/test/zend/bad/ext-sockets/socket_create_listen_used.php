<?php
    $rand = rand(1,999);
    // wrong parameter count
    $s_c_l = socket_create_listen(31330+$rand);
    var_dump($s_c_l);
    // default invocation
    $s_c_l2 = socket_create_listen(31330+$rand);
    var_dump($s_c_l2);
    socket_close($s_c_l2);
    socket_close($s_c_l);
?>