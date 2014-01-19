<?php

function get_declared_user_traits() {
  $ret = array();
  foreach (get_declared_traits() as $v) {
    // exclude system traits
    $rc = new ReflectionClass($v);
    if ('systemlib.php' !== basename($rc->getFileName())) {
      $ret[] = $v;
    }
  }
  return $ret;
}
class MY_CLASS {
 }
interface MY_INTERFACE {
 }
trait MY_TRAIT {
 }
abstract class MY_ABSTRCT_CLASS {
 }
final class MY_FINAL_CLASS {
 }
var_dump(get_declared_user_traits());
?>
