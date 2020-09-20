<?hh

<<__EntryPoint>> function main(): void {
  $s = "abc";
  $s[-1] = "x";
  $s[6] = "gx";
  $s[4] = "ex";
  var_dump($s);
}
