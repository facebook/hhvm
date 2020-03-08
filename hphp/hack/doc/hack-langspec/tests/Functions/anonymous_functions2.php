<?hh // strict

namespace NS_qq;

function main(num $p1): void
{
  echo "=========== \$doubler1 ===============\n\n";

  $doubler1 = (function ($p) { var_dump($p); return $p * 2; });
  var_dump($doubler1(3));
  var_dump($doubler1(4.2));
  var_dump($doubler1($p1));

  echo "\n=========== \$doubler2 ===============\n\n";

  $doubler2 = (function ($p) { var_dump($p); return $p * 2.0; });
  var_dump($doubler2(3));
  var_dump($doubler2(4.2));
  var_dump($doubler2($p1));

  echo "\n=========== \$doubler3 ===============\n\n";

//  $doubler3 = (function (int $p) { var_dump($p); return $p * 2; });
//  $doubler3 = (function ($p = 0) { var_dump($p); return $p * 2; });
  $doubler3 = (function ($p): int { var_dump($p); return $p * 2; });
  var_dump($doubler3(3));
//  var_dump($doubler3(4.2));	// rejected; int/float not compatible
//  var_dump($doubler3($p1));
}

/* HH_FIXME[1002] call to main in strict*/
main(10);
/* HH_FIXME[1002] call to main in strict*/
main(1.3);
