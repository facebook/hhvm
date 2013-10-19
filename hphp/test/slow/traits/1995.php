<?php

function get_declared_user_traits() {
  $ret = array();
  $system_traits = Set {
    'strictiterable',
    'strictkeyediterable',
    'lazyiterable',
    'lazykeyediterable'
  };
  foreach (get_declared_traits() as $v) {
    if (!$system_traits->contains(strtolower($v))) {
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
