<?hh
<<__EntryPoint>> function main(): void {
eval("function test() { echo \"hey, this is a function inside an eval()!\\n\"; }");

$i=0;
while ($i<10) {
  test();
  $i++;
}

eval('function test2() { -; }');
}
