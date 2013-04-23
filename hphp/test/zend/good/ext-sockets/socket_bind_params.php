<?php
    $rand = rand(1,999);
    $s_c = socket_create_listen(31330+$rand);
    $s_w = socket_bind();
    var_dump($s_w);
    $s_w = socket_bind($s_c);
    var_dump($s_w);    
    socket_close($s_c);

?>