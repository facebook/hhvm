<?hh

function heh($ar) {
  if (!$ar) throw new Exception('a');
  return 42;
}
function bar($ar) {
  $tmp = 54;
  try {
    $tmp = heh($ar);
  } catch (Exception $x) {
    var_dump($tmp);
  }
  var_dump($tmp);
}


<<__EntryPoint>>
function main_dce_003() {
bar(varray['a']);
bar(varray[]);
}
