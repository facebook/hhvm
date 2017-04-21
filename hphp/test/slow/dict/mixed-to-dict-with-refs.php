<?hh

function array_unset<Tk, Tv>(array<Tk, Tv> $arr, Tk $k): array<Tk, Tv> {
  unset($arr[$k]);
  return $arr;
}

function test($p) {
  foreach ($p as $k => &$v) {
    $v = array_unset($v, 'foo');
  }

  return \hh\dict($p);
}

var_dump(test(
           array(
             'x' => array('x' => 1,
                          'foo' => 42),
             'y' => array('x' => 2,
                          'foo' => 42),
             'z' => array('x' => 3,
                          'foo' => 42),
           )
         ));
