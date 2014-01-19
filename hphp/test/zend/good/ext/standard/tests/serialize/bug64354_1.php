<?php
spl_autoload_register(
    function($class) {
        throw new Exception("Failed");
    }
);

try {
    var_dump(unserialize('O:1:"A":0:{}'));
} catch (Exception $e) { 
    var_dump($e->getMessage());
}

try {
    var_dump(unserialize('a:2:{i:0;O:1:"A":0:{}i:1;O:1:"A":0:{}}'));
} catch (Exception $e) { 
    var_dump($e->getMessage());
}
?>