<?php
declare(ticks=1);
register_tick_function($closure = function () { echo "Tick!\n"; });
unregister_tick_function($closure);
echo "done";
?>