<?hh
function test($str) :mixed{
  echo "\n--> Testing $str:\n";
  var_dump((int)$str);
  var_dump((float)$str);
  var_dump(HH\Lib\Legacy_FIXME\gt($str, 0));
}
<<__EntryPoint>> function main(): void {
test("..9");
test(".9.");
test("9..");
test("9.9.");
test("9.9.9");
echo "===DONE===\n";
}
