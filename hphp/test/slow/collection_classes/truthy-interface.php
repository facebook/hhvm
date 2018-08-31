<?hh

function test(ConstSet<stdclass> $a) {
  var_dump(!$a);
}


<<__EntryPoint>>
function main_truthy_interface() {
test(Set {});
}
