<?hh // experimental

let x : int = 42;
{
  var_dump(x);
  let x : string = "Forty-two";
  var_dump(x);
}
var_dump(x);
