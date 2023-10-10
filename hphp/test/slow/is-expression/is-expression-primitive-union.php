<?hh

function is_arraykey(mixed $x): void {
  if ($x is arraykey) {
    echo "arraykey\n";
  } else {
    echo "not arraykey\n";
  }
}

function is_num(mixed $x): void {
  if ($x is num) {
    echo "num\n";
  } else {
    echo "not num\n";
  }
}


<<__EntryPoint>>
function main_is_expression_primitive_union() :mixed{
is_arraykey(1);
is_arraykey("one");
is_arraykey(1.5);
is_arraykey(true);

echo "---\n";

is_num(1);
is_num("one");
is_num(1.5);
is_num(true);
}
