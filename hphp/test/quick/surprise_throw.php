<?hh

// bug #2088495
function asd($x, $y) :mixed{
  if ($x == 'exit' && $y == 'foo') {
    echo "yep\n";
    throw new Exception ('yo');
  }
  echo "hi $x $y\n";
}

function foo() :mixed{
  $x = new stdClass;
  $y = new stdClass;
  $z = new stdClass;
  return new stdClass;
}
<<__EntryPoint>>
function entrypoint_surprise_throw(): void {
  fb_setprofile('asd');

  try {
    foo();
  } catch (Exception $x) {
    echo $x->getMessage() . "\n";
  }
}
