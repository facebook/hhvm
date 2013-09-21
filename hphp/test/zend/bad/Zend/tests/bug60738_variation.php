<?php

var_dump(set_exception_handler(
    function() { echo 'Intercepted exception!', "\n"; }
));

var_dump(set_exception_handler(null));

throw new Exception('Exception!');
?>