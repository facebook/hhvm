<?hh

class Dtor {
}

function do_unset($v, $k) :mixed{
  try {
    unset($v[$k]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  return $v;
}

<<__EntryPoint>> function main(): void {
  $v1 = vec[1, 2, 3];
  var_dump($v1);
  $v1 = do_unset($v1, 2);
  var_dump($v1);

  echo "====================================================\n";

  $v2 = vec['a', 'b', 'c'];
  var_dump($v2);
  $v2 = do_unset($v2, 1);
  var_dump($v2);

  echo "====================================================\n";

  $v3 = vec[true, false, null];
  var_dump($v3);
  $v3 = do_unset($v3, 100);
  var_dump($v3);

  echo "====================================================\n";

  $v4 = vec[1, 'abc', new Dtor];
  var_dump($v4);
  $v4 = do_unset($v4, 2);
  var_dump($v4);
}
