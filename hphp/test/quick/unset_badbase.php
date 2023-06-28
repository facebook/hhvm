<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

function main() :mixed{
  $a = null;
  unset($a['foo']);
  unset($o->foo['blah']);
  unset($o['basd']['sadfsadf']);
  return $a;
}
function main2() :mixed{
  $a = 'hello';
  unset($a['bar']);
}
<<__EntryPoint>> function main_entry(): void {
var_dump(main());
main2();
}
