<?hh

<<__EntryPoint>>
function main(): void {
  $s = shape('a' => shape('b' => shape('c' => 42)));
  $a = $s['b']['b']['c'] ?? 42;
}
