<?hh
function test($group) {
  $children = $group['children'];
  $count = count($children);
  $show_num = min($count, 25);
  $remaining = varray[$count - $show_num, $group['limit_hit']];
  return $remaining === varray[0, false];
}

<<__EntryPoint>>
function main_same_001() {
var_dump(test(darray['children' => varray[1,2,3], 'limit_hit' => false]));
}
