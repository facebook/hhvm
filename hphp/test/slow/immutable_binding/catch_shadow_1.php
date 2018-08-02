<?hh // experimental

function foo(): void {
  let e = 42;
  try {
    throw new Exception("Exception");
  } catch (Exception e) {
    var_dump(e->getMessage());
  }
  var_dump(e);
}

foo();
