<?hh

function f(arraykey $i): string {
  $j = HH\FIXME\UNSAFE_CAST<arraykey, string>($i); // compiles to $j = $i;
  return $j; // TypeHintViolation
}

<<__EntryPoint>>
function main(): void {
  f(3);
}
