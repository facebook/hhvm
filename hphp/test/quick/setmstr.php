<?hh

function foo(inout $str) {
  $str[3] = '.';
  $str .= "\n";
}
<<__EntryPoint>> function main(): void {
$a = "abc";
foo(inout $a);
var_dump($a);
}
