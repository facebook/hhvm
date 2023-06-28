<?hh


function main($a, $doit) :mixed{
  $o = "abc";
  if ($doit) {
    $o = darray[];
    foreach ($a as $k => $v) {
      $o[$k] = $v;
    }
  } else {
    $o = "else taken";
  }
  return $o;
}


<<__EntryPoint>>
function main_loop_poly1() :mixed{
var_dump(main(varray[], 1));
var_dump(main(varray["1",1], 1));
var_dump(main(varray["1",1], 1));
var_dump(main(varray["1",1], 0));
var_dump(main(varray["1",1,1], 0));
var_dump(main(varray["1",1], 0));
var_dump(main(varray[], 0));
}
