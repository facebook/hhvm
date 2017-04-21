<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($x) { var_dump((stdclass::$$x)[0]); }
test(42);
