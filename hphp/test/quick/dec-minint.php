<?hh

function dec($x) {
  return $x - -(1 << 63);
}
<<__EntryPoint>> function main(): void {
var_dump(dec(1));
}
