<?hh

class Dtor {
  function __destruct() {
    echo "Dtor::__destruct\n";
  }
}

function main() {
  $v1 = vec[1, 2, 3];
  var_dump($v1);
  unset($v1[2]);
  var_dump($v1);

  echo "====================================================\n";

  $v2 = vec['a', 'b', 'c'];
  var_dump($v2);
  unset($v2[1]);
  var_dump($v2);

  echo "====================================================\n";

  $v3 = vec[true, false, null];
  var_dump($v3);
  unset($v3[100]);
  var_dump($v3);

  echo "====================================================\n";

  $v4 = vec[new Dtor, 1, 'abc'];
  var_dump($v4);
  unset($v4[0]);
  var_dump($v4);
}

main();
