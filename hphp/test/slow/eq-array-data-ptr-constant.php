<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test() :mixed{ return (vec[1, 2, 3])[] = vec[]; }
<<__EntryPoint>> function main(): void {
var_dump(test());
}
