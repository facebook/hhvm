<?hh

function err() {
  return true;
}

function doswitch($foo) {
  echo "--------------------------------\n";
  var_dump($foo);
  switch($foo) {
    case 0:
      echo "0\n";
      break;

    case -1:
      echo "-1\n";
      break;

    case 1:
      echo "1\n";
      break;

    case 4:
    case 2:
      echo "2 or 4: $foo\n";
      break;

    case 5:
      echo "5\n";
      break;

    default:
      echo "default\n";
      break;
  }
}

class c {}

function main() {
  $f = fopen("/dev/null", "w");
  $things = varray[
    null,
    true,
    false,
    5,
    2.0,
    2.2,
    '2',
    '2.0',
    '2.2',
    'blar',
    '',
    4,
    varray[],
    varray['foo', 'floo'],
    new stdclass(),
    new c(),
    $f
  ];
  var_dump($f == 4);
  foreach ($things as $t) {
    doswitch($t);
  }
}
<<__EntryPoint>>
function main_entry(): void {
  set_error_handler(fun('err'));
  main();
}
