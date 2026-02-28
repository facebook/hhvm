<?hh

type Tarraykey = arraykey;
type Tnum = num;

function is_arraykey(mixed $x): void {
  if ($x is Tarraykey) {
    echo "arraykey\n";
  } else {
    echo "not arraykey\n";
  }
}

function is_num(mixed $x): void {
  if ($x is Tnum) {
    echo "num\n";
  } else {
    echo "not num\n";
  }
}


<<__EntryPoint>>
function main_is_expression_ta_primitive_union() :mixed{
is_arraykey(1);
is_arraykey("one");
is_arraykey(1.5);
is_arraykey(true);
is_arraykey(fopen(__FILE__, 'r'));
is_arraykey(new stdClass());

echo "\n";

is_num(1);
is_num("one");
is_num(1.5);
is_num(true);
is_num(fopen(__FILE__, 'r'));
is_num(new stdClass());
}
