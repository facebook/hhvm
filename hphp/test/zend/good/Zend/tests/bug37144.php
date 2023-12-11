<?hh
function foo() :mixed{
  $x = new stdClass();
  $x->bar = dict[0 => 1];
  return $x;
}
<<__EntryPoint>> function main(): void {
foo()->bar[1] = "123";
foo()->bar[0]++;
unset(foo()->bar[0]);
echo "ok\n";
}
