<?hh
function test1() :mixed{
  print __FUNCTION__;
}
function test2() :mixed{
  print __FUNCTION__;
}
<<__EntryPoint>> function main(): void {
fb_rename_function('test2', 'test3');
fb_rename_function('test1', 'test2');
test2();
fb_rename_function('test2', 'test3');
test2();
}
