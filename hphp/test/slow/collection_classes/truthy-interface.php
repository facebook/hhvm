<?hh

function test(ConstSet<stdclass> $a) {
  var_dump(!$a);
}

test(Set {});
