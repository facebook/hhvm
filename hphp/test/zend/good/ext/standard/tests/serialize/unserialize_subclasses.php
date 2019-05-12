<?php

class C {}
class D extends C {}
<<__EntryPoint>> function main() {
$c = serialize(new C);
$d = serialize(new D);

var_dump(unserialize($c, ["allowed_classes" => ["C"]]));
var_dump(unserialize($c, ["allowed_classes" => ["D"]]));
var_dump(unserialize($d, ["allowed_classes" => ["C"]]));
var_dump(unserialize($d, ["allowed_classes" => ["D"]]));
}
