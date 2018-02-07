<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.


<<a("a"."b"."c"."d"."e")>>
class C {
}

$r = new ReflectionClass("C");
var_dump($r->getAttributes());
