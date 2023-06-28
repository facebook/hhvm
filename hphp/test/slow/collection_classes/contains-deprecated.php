<?hh


<<__EntryPoint>>
function main_contains_deprecated() :mixed{
$v = Vector {0, 1, 2};
var_dump($v->contains(1));
var_dump($v->contains(5));

// contains should still work for Set though as it is not deprecated.

$s = Set {0, 1, 2};
var_dump($s->contains(1));
var_dump($s->contains(5));
}
