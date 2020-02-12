<?hh

<<__EntryPoint>>
function cgetm_ge() {
  $GLOBALS['gArray'] = array(1, 2, 'bob', 'cat');
  $GLOBALS['gStr'] = '01234567890';


  var_dump($GLOBALS['gArray'][2]);
  var_dump($GLOBALS['gStr'][3]);

  $idx = array(2, 3);
  foreach (array('gArray', 'gStr') as $dyn) {
    var_dump($GLOBALS[$dyn][array_shift(inout $idx)]);
  }

  try {
    $a = $GLOBALS['doesnt_exist'][12];
    var_dump($a);
  } catch (Exception $e) { echo $e->getMessage()."\n"; }

  foreach (HH\global_keys() as $k) {
    if ($k == 'doesnt_exist') {
      echo "has key $k\n";
    }
  }
}
