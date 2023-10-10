<?hh
// Copyright 2004-present Facebook. All Rights Reserved.


<<a("a"."b"."c"."d"."e")>>
class C {
}


<<__EntryPoint>>
function main_user_attribute_concat() :mixed{
$r = new ReflectionClass("C");
var_dump($r->getAttributes());
}
