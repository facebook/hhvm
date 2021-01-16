<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.


function reads_off_varray(varray<int> $v)[$v::C, $v::CMut]: void {}

function reads_off_darray(darray<int, int> $v)[$v::C, $v::CMut]: void {}

function reads_off_varray_or_darray(
  varray_or_darray<int> $v
)[$v::C, $v::CMut]: void {}
