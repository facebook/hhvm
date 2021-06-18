<?hh

function test(ConstSet<stdClass> $a) {
  var_dump(!$a);
}


<<__EntryPoint>>
function main_truthy_interface() {
test(Set {});
}
