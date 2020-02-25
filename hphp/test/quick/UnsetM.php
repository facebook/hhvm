<?hh

<<__EntryPoint>>
function main() {

  $a = darray[];
  $a[0] = 10;
  $a[1] = 11;
  $a["hi"] = "HI";
  $a["bye"] = "BYE";
  unset($a[1]);
  unset($a["hi"]);
  var_dump($a);
  $GLOBALS['a'] = $a;
  // Try out G bases
  $idxDefined = "foo";
  $idxNotDefined = "-- )) \\";
  $GLOBALS['a'][$idxDefined] = 071177;
  var_dump($GLOBALS['a']);
  unset($GLOBALS['a'][$idxDefined]);
  unset($GLOBALS['a'][$idxNotDefined]);
  var_dump($GLOBALS['a']);

  // Regression test for a translator bug
  $k = strtolower('blah');  // make it a dynamic string
  $s = darray[$k => 123];
  unset($s[$k]);
  unset($s[$k]);  // should have no effect
  var_dump($s);
}
