<?php

class test {
     function throwException() { throw new Exception("Hello World!\n"); 
} }

$array = array(new test(), 'throwException');
try {
     call_user_func($array, 1, 2);
} catch (Exception $e) {
     echo $e->getMessage();
}

try {
     call_user_func_array($array, array(1, 2));
} catch (Exception $e) {
     echo $e->getMessage();
}
?>