<?php
class foo {
    public $fubar = 'fubar';
}

function &foo(){
    $GLOBALS['foo'] = &new foo();
    return $GLOBALS['foo'];
}
$bar = &foo();
var_dump($bar);
var_dump($bar->fubar);
unset($bar);
$bar = &foo();
var_dump($bar->fubar);

$foo = &foo();
var_dump($foo);
var_dump($foo->fubar);
unset($foo);
$foo = &foo();
var_dump($foo->fubar);
?>