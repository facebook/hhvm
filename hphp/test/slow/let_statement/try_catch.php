<?hh // experimental

let x = 42;
try {
  var_dump(x);
  let x = "x";
  var_dump(x);
  throw new Exception();
} catch (Exception $e) {
  var_dump(x);
} finally {
  var_dump(x);
}
