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
class this_is_a_class { }
interface this_is_an_interface {
  public function this_is_an_interface_method();
}
trait this_is_a_trait { }
abstract class this_is_an_abstract_class { }
final class this_is_a_final_class { }
var_dump(get_declared_user_traits());
?>
