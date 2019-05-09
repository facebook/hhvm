<?hh // experimental
<<__EntryPoint>> function main(): void {
let identical = ($x) ==> {
  let y = $x;
  return y;
};
var_dump(identical(42));
}
