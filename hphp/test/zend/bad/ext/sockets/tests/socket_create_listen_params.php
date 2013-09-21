<?php
    $rand = rand(1,999);
    $s_c_l = socket_create_listen();
    var_dump($s_c_l);
    if ($s_c_l !== false) {
        @socket_close($s_c_l);
    }
?>