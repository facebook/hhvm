<?hh

function foo(&$str) {
  $str[3] = '.';
  $str .= "\n";
}
<<__EntryPoint>> function main(): void {
$a = "abc";
foo(&$a);
var_dump($a);
}
