<?hh

function closed_shape(): ?shape('hello' => string) {
  return shape('hello' => 'there');
}

function test_closed(): void {
  $s = closed_shape();
  echo Shapes::idx($s, 'not_a_key');        // no error, lint
  $x = $s['not_a_key'] ?? 'bli';            // no error, lint
  $s as nonnull;
  echo Shapes::idx($s, 'not_a_key');        // error, no lint
  echo Shapes::at($s, 'not_a_key');         // throw, error, no lint
  echo Shapes::keyExists($s, 'not_a_key');  // error, no lint
  echo $s['not_a_key'];                     // error, no lint
  $x = $s['not_a_key'] ?? 'bli';            // error, no lint
}

function open_shape(): ?shape('hello' => string, ...) {
  return shape('hello' => 'there');
}

function test_open(): void {
  $s = open_shape();
  echo Shapes::idx($s, 'not_a_key');        // no error, no lint
  $x = $s['not_a_key'] ?? 'bli';            // no error, no lint
  $s as nonnull;
  echo Shapes::idx($s, 'not_a_key');        // no error, no lint
  echo Shapes::at($s, 'not_a_key');         // throw, no error, no lint
  echo Shapes::keyExists($s, 'not_a_key');  // no error, no lint
  echo $s['not_a_key'];                     // error, no lint
  $x = $s['not_a_key'] ?? 'bli';            // no error, no lint
}

<<__EntryPoint>>
function main(): void {
  test_closed();
  test_open();
}
