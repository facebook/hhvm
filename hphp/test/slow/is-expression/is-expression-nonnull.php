<?hh

function is_nonnull($x): void {
  if ($x is nonnull) {
    echo "nonnull\n";
  } else {
    echo "null\n";
  }
}


<<__EntryPoint>>
function main_is_expression_nonnull() :mixed{
is_nonnull(null);
is_nonnull(-1);
is_nonnull(false);
is_nonnull(1.5);
is_nonnull('foo');
is_nonnull(fopen(__FILE__, 'r'));
is_nonnull(new stdClass());
is_nonnull(tuple(1, 2, 3));
is_nonnull(shape('a' => 1, 'b' => 2));
}
