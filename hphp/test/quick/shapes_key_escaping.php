<?hh

type ShapeKeyEscaping = shape(
  'Whomst\'d\'ve' => int,
  'Whomst\u{0027}d\u{0027}ve' => string,
);

function get_int_value(ShapeKeyEscaping $x): int {
  return $x['Whomst\'d\'ve'];
}

<<__EntryPoint>>
function main(): void {
  echo get_int_value(shape(
    'Whomst\'d\'ve' => 4,
    'Whomst\u{0027}d\u{0027}ve' => "aaa",
  ));
}
