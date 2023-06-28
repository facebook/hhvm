<?hh

function asd($x, $y, ...$z) :mixed{
  asd($x, $y, $x + $y, $y + $x, "asd");
}
<<__EntryPoint>> function main(): void {
asd(1, 2);
}
