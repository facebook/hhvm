<?php
function test($group) {
  $children = $group['children'];
  $count = count($children);
  $show_num = min($count, 25);
  $remaining = array($count - $show_num, $group['limit_hit']);
  return $remaining === array(0, false);
}
var_dump(test(array('children' => array(1,2,3), 'limit_hit' => false)));
