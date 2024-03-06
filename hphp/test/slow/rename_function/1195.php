<?hh

function test1() :mixed{
  var_dump(__METHOD__);
}
function test2() :mixed{
  var_dump(__METHOD__);
}
function test($test) :mixed{
  test1();
  test1();
  $test();
}

<<__EntryPoint>>
function main_1195(): mixed {
  test('test1');
  fb_rename_function('test1', 'fiz');
  fb_rename_function('test2', 'test1');
  test('test1');
}
