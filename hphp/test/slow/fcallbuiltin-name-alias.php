<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function decompose($s) :mixed{
  $v = vec[];
  for ($i = 0; $i < strlen($s); $i++) {
    $v[] = ord($s[$i]);
  }
  return $v;
}

function foo($x) :mixed{ return IntlChar::chr($x); }


<<__EntryPoint>>
function main_fcallbuiltin_name_alias() :mixed{
var_dump(decompose(foo(169)));
var_dump(decompose(foo(169)));
}
