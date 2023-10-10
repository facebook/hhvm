<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

final class :d:e {}

function print_xhp_class_unmangled(:d:e $xhp) {
  $xhp_cls = get_class($xhp);
  print "$xhp_cls\n";
}
