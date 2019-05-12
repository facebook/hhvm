<?hh
function foo($a, $b) {
  // reg * reg
  return $a * $b;
}
<<__EntryPoint>> function main(): void {
echo foo(5, -7);
echo "\n";
}
