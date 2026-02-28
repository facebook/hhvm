<?hh

function test() :mixed{
  $f = fopen(__FILE__, "r");
  var_export($f);
  echo "\n";
  var_dump(var_export($f, true));
}


<<__EntryPoint>>
function main_resource() :mixed{
test();
}
