<?php

abstract class Foo {
abstract public static function who();
}

function main() {
  Foo::who();
}

main();
