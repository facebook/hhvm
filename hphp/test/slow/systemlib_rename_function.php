<?hh

function test_file($file) : vec<string> {
  return HH\Lib\Vec\take(original_file($file), 2);
}

<<__EntryPoint>>
function main(): void {
  var_dump(file(__FILE__));

  fb_rename_function("file", "original_file");
  fb_rename_function("test_file", "file");
  var_dump(file(__FILE__));

  fb_rename_function("HH\Lib\Vec\map", "test_map");
  fb_rename_function("HH\Lib\Vec\map_async", "test_map1");
}
