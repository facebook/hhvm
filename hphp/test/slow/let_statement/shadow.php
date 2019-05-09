<?hh // experimental
<<__EntryPoint>> function main(): void {
let x = 42;
{
  var_dump(x);
  let x = -1;
  var_dump(x);
}
var_dump(x);
}
