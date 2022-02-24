<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function testforeach2(dynamic $d):void {
  if (HH\is_any_array($d)) {
    foreach ($d as $y) {
      $x = $y[0];
    }
  }
}
