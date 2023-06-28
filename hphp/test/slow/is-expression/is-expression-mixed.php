<?hh

function is_mixed($x): void {
  if ($x is mixed) {
    echo "mixed\n";
  } else {
    echo "not mixed\n";
  }
}


<<__EntryPoint>>
function main_is_expression_mixed() :mixed{
is_mixed(null);
is_mixed(-1);
is_mixed(false);
is_mixed(1.5);
is_mixed('foo');
is_mixed(fopen(__FILE__, 'r'));
is_mixed(new stdClass());
is_mixed(tuple(1, 2, 3));
is_mixed(shape('a' => 1, 'b' => 2));
}
