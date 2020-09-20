<?hh

const int FOO = 1;
const int BAR = A::BAR;
const int QUX = BAZ;

class A {
  const int BAR = 2;
}

<<__EntryPoint>>
function main(): void {
  HH\autoload_set_paths(
    dict[
      'constant' => dict[
        'BAZ' => 'constant_folding.inc',
      ],
    ],
    __DIR__.'/',
  );

  var_dump(FOO);
  var_dump(BAR);
  var_dump(BAZ);
  var_dump(QUX);
}
