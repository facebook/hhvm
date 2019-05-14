<?php <<__EntryPoint>> function main() {
$rc = new ReflectionFunction('ereg');
var_dump($rc->isDeprecated());
}
