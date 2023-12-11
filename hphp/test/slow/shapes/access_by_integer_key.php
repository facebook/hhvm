<?hh

enum KeyType: int as int {
  ZERO = 0;
  ONE = 1;
}

type Tshape = shape(
  KeyType::ZERO => int,
  KeyType::ONE => string,
);

function f(Tshape $s): string {
  var_dump($s[0]);
  var_dump($s[1]);
  return $s[1];
}

enum KeyType2: arraykey as arraykey {
  ZERO = 'zero';
  ONE = 'one';
  TWO = 2;
}

type Tshape2 = shape(
  KeyType::ZERO => int,
  KeyType::ONE => string,
  KeyType::TWO => float,
);

function g(Tshape $s): float {
  var_dump($s['zero']);
  var_dump($s['one']);
  var_dump($s[2]);
  return $s[2];
}
<<__EntryPoint>>
function main_entry(): void {

  f(dict[0 => 0, 1 => 'one']);

  g(dict['zero' => 0, 'one' => 'one', 2 => 2.]);
}
