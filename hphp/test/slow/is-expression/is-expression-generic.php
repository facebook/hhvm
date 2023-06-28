<?hh

function f<T>(mixed $x): void {
  if ($x is T) {
    echo "T\n";
  } else {
    echo "not T\n";
  }
}


<<__EntryPoint>>
function main_is_expression_generic() :mixed{
f(42);
}
