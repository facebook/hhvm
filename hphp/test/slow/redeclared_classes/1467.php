<?hh

function f($i) :mixed{
  $j = 1;
  var_dump($j);
  if ($i == 1) {
    include '1467-1.inc';
  } else {
    include '1467-2.inc';
  }
}
<<__EntryPoint>>
function entrypoint_1467(): void {
  try {
    if ($i == 1) {
      include '1467-3.inc';
    }
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  f(1);
  $obj = new p();
  var_dump($obj);
  $obj = new c();
  var_dump($obj);
}
