<?hh

function foo() :mixed{
  $e = new Exception();
  try {
    throw new Exception2();
  }
  catch (Exception $e) {
    var_dump($e->getCode());
  }
}
<<__EntryPoint>>
function entrypoint_1461(): void {

  $b = 123;
  if ($b) {
    include '1461-1.inc';
  } else {
    include '1461-2.inc';
  }

  include '1461-classes.inc';

  foo();
}
