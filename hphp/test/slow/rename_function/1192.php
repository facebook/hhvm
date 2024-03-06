<?hh

function test1(): mixed {
  echo "test1\n";
}
function test3(): mixed {
  echo "test3\n";
}
function dump($test1, $test2) {
  var_dump(function_exists("test1"));
  var_dump(function_exists("test2"));
  var_dump(function_exists($test1));
  var_dump(function_exists($test2));
}

<<__EntryPoint>>
function main_1192(): mixed {
  dump("test1", "test2");
  fb_rename_function("test1", "test2");
  dump("test1", "test2");
  fb_rename_function("test3", "test1");
  dump("test1", "test2");
  test1();
  test2();
}
