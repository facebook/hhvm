<?hh // strict

function nullthrows<T>(?T $x, ?string $message = null, ...$sprintf_args): T {
  if ($x !== null) {
    return $x;
  }

  if ($message === null) {
    $message = 'Got unexpected null';
  }

  throw new Exception($message);
}


function main($obj) {
  var_dump(nullthrows($obj));
}

class C { }

$obj = new C;

main($obj);
main($obj);
main($obj);
main($obj);
main($obj);
