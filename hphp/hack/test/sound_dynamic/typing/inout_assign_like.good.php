<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function get():~int { return 3; }

function put(inout ~int $s):void {
  $s = get();
}
