<?hh

// A keyset keeps an element at the position of its first insertion; a later
// duplicate is a no-op. When HHBBC knows the arguments it rebuilds the keyset
// itself, and used to place the duplicated element where it last appeared.
// The literal form was always folded correctly, so both are checked.

function build(string $a, string $b): keyset<string> {
  return keyset[$a, $b, $a];
}

function show(keyset<string> $k): void {
  foreach ($k as $v) {
    echo '[', $v, ']';
  }
  echo "\n";
}

<<__EntryPoint>>
function main_keyset_duplicate_order(): void {
  show(build("", "x"));
  show(build("a", "b"));
  show(keyset["", "x", ""]);
}
