<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test() { return (vec[1, 2, 3])[] = vec[]; }
var_dump(test());
