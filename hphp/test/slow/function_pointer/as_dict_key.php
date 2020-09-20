<?hh

function foo<reify T>(): void {}

<<__EntryPoint>>
function main(): void {
  $d = dict[];
  $f = foo<int>;
  try {
    $d[$f];
  } catch (Exception $e) {
    echo $e->getMessage();
  }
}
