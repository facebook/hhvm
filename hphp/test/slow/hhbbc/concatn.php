<?hh

function f(string $x, string $y, string $z): string {
  return $x.$y.$z;
}
<<__EntryPoint>> function main(): void {
echo f('a', 'b', 'c');
echo "\n";
}
