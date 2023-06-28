<?hh

function is_null_($x): void {
  if ($x is null) {
    echo "null\n";
  } else {
    echo "not null\n";
  }
}

function return_void(): void {}


<<__EntryPoint>>
function main_is_expression_null() :mixed{
is_null_(null);
is_null_(return_void());
is_null_(-1);
is_null_(false);
is_null_(1.5);
is_null_('foo');
is_null_(fopen(__FILE__, 'r'));
is_null_(new stdClass());
is_null_(tuple(1, 2, 3));
is_null_(shape('a' => 1, 'b' => 2));
}
