<?hh

function basic() :mixed{
  echo '=== ', __FUNCTION__, " ===\n";
  $os = Map { 'osx' => 'Mac', 'win' => 'NT',
              'irx' => 'Irix', 'lnx' => 'Linux' };
  var_dump(in_array('Irix', $os));
  var_dump(array_search('Irix', $os));
  var_dump(in_array('osx', $os));
  var_dump(array_search('osx', $os));
}

function strict() :mixed{
  echo '=== ', __FUNCTION__, " ===\n";
  $s = Set { '110', 124, 113 };
  var_dump(in_array('124', $s));
  var_dump(array_search('124', $s));
  var_dump(in_array('124', $s, true));
  var_dump(array_search('124', $s, true));
  var_dump(in_array(113, $s, true));
  var_dump(array_search(113, $s, true));
}

function nested() :mixed{
  echo '=== ', __FUNCTION__, " ===\n";
  $a = Vector { vec['p', 'h'], vec['p', 'r'], 'o' };
  var_dump(in_array(vec['p', 'h'], $a));
  var_dump(array_search(vec['p', 'h'], $a));
  var_dump(in_array(vec['f', 'i'], $a));
  var_dump(array_search(vec['f', 'i'], $a));
  var_dump(in_array('o', $a));
  var_dump(array_search('o', $a));
}


<<__EntryPoint>>
function main_array_search_funcs() :mixed{
basic();
strict();
nested();
}
