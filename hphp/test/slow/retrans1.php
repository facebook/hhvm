<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main($a, $b, $c) :mixed{
  return HH\Lib\Legacy_FIXME\cast_for_arithmetic($a) + HH\Lib\Legacy_FIXME\cast_for_arithmetic($b) + HH\Lib\Legacy_FIXME\cast_for_arithmetic($c);
}
<<__EntryPoint>> function main_entry(): void {
for ($i=0; $i < 5; $i++) {
  var_dump(main(1, 2, 3));
  var_dump(main(1, 2, 3.5));
  var_dump(main(1, 2.5, 3));
  var_dump(main(1, 2.5, 3.5));
  var_dump(main(1.5, 2, 3));
  var_dump(main(1.5, 2, 3.5));
  var_dump(main(1.5, 2.5, 3));
  var_dump(main(1.5, 2.5, 3.5));
}
var_dump(main(true, true, true));
}
