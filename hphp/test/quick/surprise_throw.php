<?hh

# bug #2088495
function asd($x, $y) {
  if ($x == 'exit' && $y == 'foo') {
    echo "yep\n";
    throw new Exception ('yo');
  }
  echo "hi $x $y\n";
}

function foo() {
  $x = new stdclass;
  $y = new stdclass;
  $z = new stdclass;
  return new stdclass;
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
