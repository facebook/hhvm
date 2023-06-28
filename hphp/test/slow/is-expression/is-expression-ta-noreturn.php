<?hh

type Tnoreturn = noreturn;

function is_noreturn($x): void {
  if ($x is Tnoreturn) {
    echo "noreturn\n";
  } else {
    echo "not noreturn\n";
  }
}


<<__EntryPoint>>
function main_is_expression_ta_noreturn() :mixed{
is_noreturn(null);
is_noreturn(-1);
is_noreturn(false);
is_noreturn(1.5);
is_noreturn('foo');
is_noreturn(fopen(__FILE__, 'r'));
is_noreturn(new stdClass());
is_noreturn(tuple(1, 2, 3));
is_noreturn(shape('a' => 1, 'b' => 2));
}
