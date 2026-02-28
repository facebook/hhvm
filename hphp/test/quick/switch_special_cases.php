<?hh

function doswitch($foo) :mixed{
  echo "--------------------------------\n";
  var_dump($foo);
  switch(HH\Lib\Legacy_FIXME\int_cast_for_switch($foo, -1)) {
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
      $foo__str = (string)($foo);
      echo "2 or 4: $foo__str\n";
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

<<__EntryPoint>>
function main() :mixed{
  $f = fopen("/dev/null", "w");
  $things = vec[
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
    vec[],
    vec['foo', 'floo'],
    new stdClass(),
    new c(),
    $f
  ];
  var_dump(HH\Lib\Legacy_FIXME\eq($f, 4));
  foreach ($things as $t) {
    try {
      doswitch($t);
    } catch (Exception $e) {
      var_dump($e->getMessage());
    }
  }
}
