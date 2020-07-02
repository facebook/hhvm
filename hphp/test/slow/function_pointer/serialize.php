<?hh

function foo<reify T>(): void {}

<<__EntryPoint>>
function main(): void {
  $f = foo<int>;

  try {
    var_dump($f);
  } catch (Exception $e) {
    echo $e->getMessage();
  }
}
