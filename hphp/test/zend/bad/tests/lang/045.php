<?php
set_time_limit(1);
register_shutdown_function("plop");

function plop() {
    $ts = time();
    while(true) {
        if ((time()-$ts) > 2) {
            echo "Failed!";
            break;
        }
    }
}
plop();
?>
===DONE===