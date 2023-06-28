<?hh

function test($a) :mixed{
  fb_enable_code_coverage();
  var_dump(array_map(null, $a));
  fb_disable_code_coverage();
}


<<__EntryPoint>>
function main_hhas_coverage() :mixed{
test(varray[1,2,3]);
}
