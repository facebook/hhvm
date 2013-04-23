<?php
    $s_s = socket_strerror();
    for ($i=0;$i<=132;$i++) {
        var_dump(socket_strerror($i));
    }
?>