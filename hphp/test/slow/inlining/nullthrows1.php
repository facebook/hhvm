<?hh

function nullthrows<T>(?T $x, ?string $message = null, ...$sprintf_args): T {
  if ($x !== null) {
    return $x;
  }

  if ($message === null) {
    $message = 'Got unexpected null';
  }

  throw new Exception($message);
}


function main($obj) :mixed{
  var_dump(nullthrows($obj));
}

class C { }


<<__EntryPoint>>
function main_nullthrows1() :mixed{
$obj = new C;

main($obj);
main($obj);
main($obj);
main($obj);
main($obj);
}
