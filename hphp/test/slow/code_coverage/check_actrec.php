<?hh

function test($t) :mixed{
  fb_enable_code_coverage();
  var_dump($t);
  $coverage = fb_disable_code_coverage();
  echo "Coverage:\n";
  var_dump(idx($coverage,__FILE__,dict[]));
}


<<__EntryPoint>>
function main_check_actrec() :mixed{
test("hello");
}
