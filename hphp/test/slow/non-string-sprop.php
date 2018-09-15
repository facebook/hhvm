<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($x) { var_dump((stdclass::$$x)[0]); }

<<__EntryPoint>>
function main_non_string_sprop() {
test(42);
}
