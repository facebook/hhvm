<?hh
<<__EntryPoint>> function main(): void {
$x = null;
$foo = 'foo';
if (false) {
  $x?->$foo; // parse error
}
}
