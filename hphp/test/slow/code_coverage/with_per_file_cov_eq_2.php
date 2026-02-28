<?hh

function test($a) :mixed{
  fb_enable_code_coverage();
  var_dump(array_map(null, $a));
  $coverage = fb_disable_code_coverage();
  echo "Coverage:\n";
  var_dump(idx($coverage,__FILE__,dict[]));
}


<<__EntryPoint>>
function main_hhas_coverage() :mixed{
test(vec[1,2,3]);
}
