<?php
@session_start();
        
$foo = $_SESSION['foo'];
$bar = $_SESSION['bar'];
                        
var_dump($foo, $bar, $_SESSION);

$_SESSION['foo'] = $foo;
$_SESSION['bar'] = $bar;
                                        
var_dump($_SESSION);
?>