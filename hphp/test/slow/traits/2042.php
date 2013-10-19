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

class a {
 }
interface b {
 }
trait c {
 }
abstract class d {
 }
final class e {
 }

var_dump(get_declared_user_traits());

?>
