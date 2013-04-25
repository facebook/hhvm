<?php
    $rand = rand(1,999);
    $s_c     = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    $s_bind  = socket_bind($s_c, '0.0.0.0', 31330+$rand);
    var_dump($s_bind);
    
    // Connect to destination address
    $s_conn  = socket_getsockname($s_c, $ip, $port);
    var_dump($s_conn);
    var_dump($ip);
    var_dump($port);
    socket_close($s_c);
?>
