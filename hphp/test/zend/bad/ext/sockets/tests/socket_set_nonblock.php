<?php
    $rand = rand(1,999);
    $s_c_l = socket_create_listen(31330+$rand);
    socket_set_nonblock($s_c_l);
    var_dump($s_c_l);
    #socket_accept($s_c_l);
    socket_close($s_c_l);
?>
