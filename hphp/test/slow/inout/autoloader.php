<?hh

function loader($kind, $name) {
  echo "autoloading $kind: $name\n";
  if ($name === 'partial') {
    include 'autoloader1.inc';
  } else if ($name === 'hit1') {
    include 'autoloader2.inc';
  } else if ($name === 'hit2') {
    include 'autoloader3.inc';
  } else if ($name === 'hit3') {
    include 'autoloader4.inc';
  }
}

function main() {
  HH\autoload_set_paths(darray['function' => varray[], 'failure' =>'loader'], '');

  $v = null;
  hit1(inout $v);
  var_dump($v);
  hit2(inout $v);
  var_dump($v);
  hit3(inout $v);
  var_dump($v);

  partial(inout $v); // error
}

main();
