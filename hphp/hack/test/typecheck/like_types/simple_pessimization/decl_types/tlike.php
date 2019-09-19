<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f(~int $x): void {
  hh_show($x);
}

// Have to make inner types like as well
function deep_untrust(~(int, int) $x): void {
  hh_show($x);
}
