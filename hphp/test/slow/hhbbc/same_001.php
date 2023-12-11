<?hh
function test($group) :mixed{
  $children = $group['children'];
  $count = count($children);
  $show_num = min($count, 25);
  $remaining = vec[$count - $show_num, $group['limit_hit']];
  return $remaining === vec[0, false];
}

<<__EntryPoint>>
function main_same_001() :mixed{
var_dump(test(dict['children' => vec[1,2,3], 'limit_hit' => false]));
}
