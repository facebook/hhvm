<?hh

type Tnothing = nothing;

function is_nothing($x): void {
  if ($x is Tnothing) {
    echo "nothing\n";
  } else {
    echo "not nothing\n";
  }
}


<<__EntryPoint>>
function main_is_expression_ta_nothing() {
is_nothing(null);
is_nothing(-1);
is_nothing(false);
is_nothing(1.5);
is_nothing('foo');
is_nothing(STDIN);
is_nothing(new stdClass());
is_nothing(tuple(1, 2, 3));
is_nothing(shape('a' => 1, 'b' => 2));
}
