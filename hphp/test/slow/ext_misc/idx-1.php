<?hh

function main() :mixed{
  $o = new stdClass();
  $v = Vector { 'a' , 'b' };
  $m = Map { 'a' => 2, 'b' => 'c' };
  $s = 'hello';

  // Arrays
  var_dump(idx(dict[2 => 'h', 3 => 'i', 4 => 'j'], 4, null));
  var_dump(idx(dict['hello' => 42], 'hello', 31337));
  var_dump(idx(dict[2 => false], 2, true));
  var_dump(idx(dict['world' => 1], 'hello', $o));
  var_dump(idx(dict[2 => null], 2, 'not_reached'));
  var_dump(idx(vec[], 2, 'not_reached'));
  var_dump(idx(vec[], 'absent'));
  var_dump(idx(vec[], null, 5));
  echo "\n";

  // Collections
  var_dump(idx($m, 'a', 4));
  var_dump(idx($m, 'c', 'f'));
  var_dump(idx($m, 'absent'));
  var_dump(idx($m, null, 'f'));
  var_dump(idx($v, 0, 'd'));
  var_dump(idx($v, 2, 'd'));
  var_dump(idx($v, 31337));
  var_dump(idx($v, null, 'd'));
  echo "\n";

  // strings
  var_dump(idx($s, 1, 'abc'));
  var_dump(idx($s, 5, 'abc'));
  var_dump(idx($s, '1'));
  var_dump(idx($s, '5'));
  var_dump(idx($s, false));
  var_dump(idx($s, true));
  var_dump(idx($s, 1.0));
  var_dump(idx($s, 5.0));
  var_dump(idx($s, 'wtf'));
  var_dump(idx($s, 8));
  var_dump(idx($s, null, 'abc'));
  echo "\n";

  // null (because PHP)
  var_dump(idx(null, 'absent'));
  var_dump(idx(null, 'not_reached', 'wtf'));
  echo "\n";

  // invalid array key
  try { var_dump(idx(dict[2 => $o], $o)); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(idx(vec[$o], $o)); } catch (Exception $e) { var_dump($e->getMessage()); }

  // too few arguments
  try { var_dump(idx($s)); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(idx()); } catch (Exception $e) { var_dump($e->getMessage()); }
}


<<__EntryPoint>>
function main_idx_1() :mixed{
main();
}
