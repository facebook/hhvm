<?hh

function test(ConstSet<stdClass> $a) :mixed{
  var_dump(!$a);
}


<<__EntryPoint>>
function main_truthy_interface() :mixed{
test(Set {});
}
