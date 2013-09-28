<?php

var_dump(set_error_handler(
    function() { echo 'Intercepted error!', "\n"; }
));

trigger_error('Error!');

var_dump(set_error_handler(null));

trigger_error('Error!');
?>