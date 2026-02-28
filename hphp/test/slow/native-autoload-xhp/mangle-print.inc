<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

final class :a:b-c {}

function print_xhp_class(:a:b-c $xhp) {
  $xhp_cls = get_class($xhp);
  print "$xhp_cls\n";
}
