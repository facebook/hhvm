<?hh // strict

namespace NS_MyRangeException_test1;

require_once 'MyRangeException.php';

function main(): void {
  $re = new \NS_MyRangeException\MyRangeException("xxx", 5, 20, 30);
  var_dump($re);

  echo "=======\n";

  echo "\$re = >$re<\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
