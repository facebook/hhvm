<?php

$str = '';
var_dump($str[0] = 'a');
var_dump($str);

$str = '';
var_dump($str[5] = 'a');
var_dump($str);

$str = '';
var_dump($str[-1] = 'a');
var_dump($str);

$str = '';
var_dump($str['foo'] = 'a');
var_dump($str);

$str = '';
try {
    var_dump($str[] = 'a');
} catch (Error $e) {
    echo "Error: {$e->getMessage()}\n";
}
var_dump($str);

$str = '';
try {
    var_dump($str[0] += 1);
} catch (Error $e) {
    echo "Error: {$e->getMessage()}\n";
}
var_dump($str);

$str = '';
try {
    var_dump($str[0][0] = 'a');
} catch (Error $e) {
    echo "Error: {$e->getMessage()}\n";
}
var_dump($str);

?>
