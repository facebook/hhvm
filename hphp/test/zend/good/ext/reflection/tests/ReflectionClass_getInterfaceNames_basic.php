<?php
interface Foo { }

interface Bar { }

class Baz implements Foo, Bar { }
<<__EntryPoint>> function main() {
$rc1 = new ReflectionClass("Baz");
var_dump($rc1->getInterfaceNames());
}
