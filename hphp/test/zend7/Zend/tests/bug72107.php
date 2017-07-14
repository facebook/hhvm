<?php
set_error_handler('func_get_args');
function test($a) {
    echo $undef;
}
test(1);
?>
