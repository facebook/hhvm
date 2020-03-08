<?hh // strict

namespace NS_serialize;

require_once 'Point.php';

function main(): void {
  $p = new \Graphics\Point(2.0, 5.0);
  echo "\$p: $p\n";

  $s = serialize($p);
  var_dump($s);

  $v = unserialize($s);
  var_dump($v);
}

/* HH_FIXME[1002] call to main in strict*/
main();
