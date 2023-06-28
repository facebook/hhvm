<?hh

function is_this(mixed $x): void {
  if ($x is this) {
    echo "this\n";
  } else {
    echo "not this\n";
  }
}

<<__EntryPoint>>
function main() :mixed{
  is_this(new stdClass());
}
