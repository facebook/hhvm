<?hh


function main($a, $doit) {
  $o = "abc";
  if ($doit) {
    $o = array();
    foreach ($a as $k => $v) {
      $o[$k] = $v;
    }
  } else {
    $o = "else taken";
  }
  return $o;
}

var_dump(main(array(), 1));
var_dump(main(array("1",1), 1));
var_dump(main(array("1",1), 1));
var_dump(main(array("1",1), 0));
var_dump(main(array("1",1,1), 0));
var_dump(main(array("1",1), 0));
var_dump(main(array(), 0));
