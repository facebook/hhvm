<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main(): void {
  $collator = new Collator('en');
  $arr = darray['x' => 'pear', 'y' => 'apple'];
  $collator->asort(inout $arr);
  var_dump($arr);
  var_dump(is_darray($arr));
  $arr = darray['x' => 'pear', 'y' => 'apple'];
  $collator->sort(inout $arr);
  var_dump($arr);
  var_dump(is_varray($arr));
}
