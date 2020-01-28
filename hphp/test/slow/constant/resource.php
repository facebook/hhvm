<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

// const BAR1 = FOO1;
// const BAR2 = [FOO1, FOO1];
// const BAR3 = vec[FOO1, FOO1];
// const BAR4 = dict[1 => FOO1, 'abc' => FOO1];

const BAR5 = STDIN;
const BAR6 = varray[STDIN, STDIN];
const BAR7 = vec[STDIN, STDIN];
const BAR8 = dict[1 => STDIN, 'abc' => STDIN];

// var_dump(FOO1);
// var_dump(FOO2);
//
// var_dump(BAR1);
// var_dump(BAR2);
// var_dump(BAR3);
// var_dump(BAR4);
<<__EntryPoint>> function main(): void {
var_dump(BAR5);
var_dump(BAR6);
var_dump(BAR7);
var_dump(BAR8);
}
