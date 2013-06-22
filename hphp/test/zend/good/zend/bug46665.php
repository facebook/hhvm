<?php

$baz = '\\Foo\\Bar\\Baz';
new $baz();
function __autoload($class) {
    var_dump($class);
    require __DIR__ .'/bug46665_autoload.inc';
}

?>