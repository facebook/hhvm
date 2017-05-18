<?hh

function main() {
  $v = vec[1, 2, 3];
  var_dump($v);
  uksort($v, ($p1, $p2) ==> {
      return $p1 <=> $p2;
    });
  var_dump($v);
  return $v;
}

var_dump(main());
