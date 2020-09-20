<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function decompose($s) {
  $v = vec[];
  for ($i = 0; $i < strlen($s); $i++) {
    $v[] = ord($s[$i]);
  }
  return $v;
}

function foo($x) { return IntlChar::chr($x); }


<<__EntryPoint>>
function main_fcallbuiltin_name_alias() {
var_dump(decompose(foo(169)));
var_dump(decompose(foo(169)));
}
