<?php

abstract class foo { }

class_alias('foo', "\0");
<<__EntryPoint>> function main() {
$a = "\0";

new $a;
}
