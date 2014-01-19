<?php
    $rand = rand(1,999); 
    $s_c = socket_create_listen(31330+$rand);
    // wrong parameter count
    $s_w = socket_connect();
    $s_w = socket_connect($s_c);
    $s_w = socket_connect($s_c, '0.0.0.0');
    $s_w = socket_connect($s_c, '0.0.0.0', 31330+$rand);
    
    socket_close($s_c);

?>