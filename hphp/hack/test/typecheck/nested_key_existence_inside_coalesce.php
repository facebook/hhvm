<?hh

<<__EntryPoint>>
function main(): void {
  $s = shape('a' => shape('b' => 42));
  $a = $s['c']['b'] ?? 42;
}
