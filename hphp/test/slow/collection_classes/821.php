<?php

function main() {
  $s1 = StableMap {'a' => 1, 'b' => 2};
  $s2 = StableMap {'b' => 2, 'a' => 1};
  var_dump($s1 == $s2);
  $s2->remove('b');
  $s2['b'] = 2;
  var_dump($s1 == $s2);
  $s1['b'] = "2";
  var_dump($s1 == $s2);
  $s1['b'] = 3;
  var_dump($s1 == $s2);

  echo "------------------------\n";
  $sm = StableMap {'a' => 1, 'b' => 2};
  $m = Map {'b' => 2, 'a' => 1};
  var_dump($sm == $m);
  var_dump($m == $sm);
  $m['c'] = 3;
  var_dump($m == $sm);

  echo "------------------------\n";
  $m = StableMap {};
  var_dump($m == null);
  var_dump($m == false);
  var_dump($m == true);
  var_dump($m == 1);
  var_dump($m == "StableMap");

  echo "------------------------\n";
  $m = StableMap {'x' => 7};
  var_dump($m == null);
  var_dump($m == false);
  var_dump($m == true);
  var_dump($m == 1);
  var_dump($m == "StableMap");
}
main();
