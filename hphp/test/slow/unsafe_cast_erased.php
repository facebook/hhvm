<?hh

function f(int $i): string {
  $j = HH\FIXME\UNSAFE_CAST<int, string>($i); // compiles to $j = $i;
  return $j; // TypeHintViolation
}

<<__EntryPoint>>
function main(): void {
  f(3);
}
