<?hh

function f(string $s): void {
  try {
    $p = HH\classname_to_class($s);
    var_dump($p);
  } catch (InvalidArgumentException $ex) {
    echo $ex->getMessage();
  }
}

<<__EntryPoint>>
function main(): void {
  f("A");
}
