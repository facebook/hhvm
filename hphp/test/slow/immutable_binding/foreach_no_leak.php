<?hh // experimental

let x = 42;
let arr = [1, 2, 3];
foreach (arr as element) {
  let x = (string) element;
}
var_dump(x);
