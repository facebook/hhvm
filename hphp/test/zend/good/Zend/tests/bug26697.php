<?php

function __autoload($name)
{
    echo __METHOD__ . "($name)\n";
    var_dump(class_exists('NotExistingClass'));
    echo __METHOD__ . "($name), done\n";
}
<<__EntryPoint>> function main() {
var_dump(class_exists('NotExistingClass'));

echo "===DONE===\n";
}
