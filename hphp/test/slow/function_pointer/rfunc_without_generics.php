<?hh

function foo<reify T>(): void {
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
}

<<__EntryPoint>>
function main(): void {
  $f = foo<>;

  try {
    $f();
  } catch (Exception $e) {
    echo $e->getMessage();
  }
}
