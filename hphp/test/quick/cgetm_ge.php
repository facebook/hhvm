<?hh

<<__EntryPoint>>
function cgetm_ge() {
  $GLOBALS['gArray'] = varray[1, 2, 'bob', 'cat'];
  $GLOBALS['gStr'] = '01234567890';


  var_dump($GLOBALS['gArray'][2]);
  var_dump($GLOBALS['gStr'][3]);

  $idx = varray[2, 3];
  foreach (varray['gArray', 'gStr'] as $dyn) {
    var_dump($GLOBALS[$dyn][array_shift(inout $idx)]);
  }

  $k = 'doesnt_exist';
  try {
    $a = $GLOBALS[$k][12];
    var_dump($a);
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }

  if (HH\global_key_exists($k)) {
    echo "has key $k\n";
  }
}
