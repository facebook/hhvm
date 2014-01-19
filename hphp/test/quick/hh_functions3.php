<?hh
// Copyright 2004-2013 Facebook. All Rights Reserved.

class :x:frag { }

function foo(:x:frag $x): :x:frag {
  return $x;
}
print_r(foo(<x:frag/>));
