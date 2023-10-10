<?hh
<<__EntryPoint>> function main(): void {
$x = null;
if (false) {
  unset($x?->y); // parse error
}
}
