<?php
register_shutdown_function(function(){echo("\n\n!!!shutdown!!!\n\n");});
set_error_handler(function($errno, $errstr, $errfile, $errline){throw new Exception("Foo");});

class Bad {
    public function __toString() {
        throw new Exception('Oops, I cannot do this');
    }
}

$bad = new Bad();
echo "$bad";