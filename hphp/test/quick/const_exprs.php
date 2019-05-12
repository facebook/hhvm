<?hh

const FOO = -(10+10);
const BAR = (10+10) < (10*10);
const BAZ = 2 ** 10;
<<__EntryPoint>> function main(): void {
var_dump(FOO);
var_dump(BAR);
var_dump(BAZ);
}
