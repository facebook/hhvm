<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function my_main(): void {
  $items = vec[1, 2, 3];
  foreach ($items as $item) {
    try {
      echo "hello";
    } finally {
      continue;
    }
  }
}
