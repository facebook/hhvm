<?hh

function test($g) {
  $GLOBALS['g'] = $GLOBALS['GLOBALS'];

  array_replace_recursive($GLOBALS['GLOBALS'], $g);

  $GLOBALS['g'] = $GLOBALS['GLOBALS'];
  array_merge_recursive($GLOBALS['GLOBALS'], $g);
}

<<__EntryPoint>>
function main_self_recursive() {
  $a = darray['g' => $GLOBALS['GLOBALS']];

  test($a);
  test($GLOBALS['GLOBALS']);
}
