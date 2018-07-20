<?hh

function loader($kind, $name) {
  echo "autoloading $kind: $name\n";
  if ($name === 'partial') {
    function partial($x) {
      echo "whoops!\n";
    }
  } else if ($name === 'hit1') {
    function hit1(inout $x) {
      $x = 'hit1';
    }
  } else if ($name === 'hit2') {
    function hit2(inout $x) {
      $x = 'hit2';
    }
  } else if ($name === 'hit3') {
    function hit3(&$x) {
      $x = 'hit3';
    }
  }
}

function main() {
  HH\autoload_set_paths(array('function' => array(), 'failure' =>'loader'), '');

  $v = null;
  hit1(inout $v);
  var_dump($v);
  hit2(&$v);
  var_dump($v);
  hit3(inout $v);
  var_dump($v);

  partial(inout $v); // error
}

main();
