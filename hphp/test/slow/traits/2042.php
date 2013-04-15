<?php


function get_declared_user_traits() {
  $ret = array();
  foreach (get_declared_traits() as $v) {
    $lv = strtolower($v);
    if ($lv !== 'iterabletrait' && $lv !== 'keyediterabletrait') {
      $ret[] = $v;
    }
  }
  return $ret;
}

class a { }
interface b { }
trait c { }
abstract class d { }
final class e { }

var_dump(get_declared_user_traits());

?>
