<?hh

function f(int $i): string {
  unsafe_cast(); // ignored
  $j = unsafe_cast<int, string>($i); // compiles to $j = $i;
  return $j; // TypeHintViolation
}

<<__EntryPoint>>
function main(): void {
  f(3);
}
