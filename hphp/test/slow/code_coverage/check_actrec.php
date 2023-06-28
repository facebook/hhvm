<?hh

function test($t) :mixed{
  fb_enable_code_coverage();
  var_dump($t);
  fb_disable_code_coverage();
}


<<__EntryPoint>>
function main_check_actrec() :mixed{
test("hello");
}
