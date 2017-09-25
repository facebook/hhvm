<?hh // strict

function nullthrows<T>(?T $x, ?string $message = null, mixed ...$sprintf_args): T {
  if ($x !== null) {
    return $x;
  }

  if ($message === null) {
    $message = 'Got unexpected null';
  }

  throw new Exception($message);
}


function main(C $obj): void {
  var_dump(nullthrows($obj));
}

class C { }

$obj = new C();

main($obj);
main($obj);
main($obj);
main($obj);
main($obj);
