<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

class :x:frag { }

function foo(:x:frag $x): :x:frag {
  return $x;
}
<<__EntryPoint>> function main(): void {
print_r(foo(<x:frag/>));
}
