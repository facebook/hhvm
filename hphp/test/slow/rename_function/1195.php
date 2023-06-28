<?hh

function test1() :mixed{
  var_dump(__METHOD__);
}
function test2() :mixed{
  var_dump(__METHOD__);
}
function test($test) :mixed{
  test1();
  TeSt1();
  $test();
  $test = strtolower($test);
  $test(1,2,3);
}

<<__EntryPoint>>
function main_1195() :mixed{
test('Test1');
fb_rename_function('tEst1', 'fiz');
fb_rename_function('test2', 'Test1');
test('teSt1');
}
