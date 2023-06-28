<?hh

class C {}

<<__EntryPoint>>
function main() :mixed{
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
    new C(),
    fopen("/dev/null", "w"),
  ];
  foreach ($things as $t) {
    try {
      doswitch($t);
    } catch (Exception $e) {
      echo $e->getMessage()."\n";
    }
  }
}

function doswitch($foo) :mixed{
  echo "--------------------------------\n";
  var_dump($foo);
  switch ($foo) {
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
      $foo = (string)$foo;
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
