<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function badly_located(
  bool $b,
  (function(string): Map<string, mixed>) $f,
): void {
  $list = Vector {};
  if ($b) {
    $item = $list->map($f);
  } else {
    $item = $list;
  }
}
