<?hh // experimental
<<__EntryPoint>> function main(): void {
let amount = 10;
let incr = ($x) ==> $x + amount;
var_dump(incr(10));
}
