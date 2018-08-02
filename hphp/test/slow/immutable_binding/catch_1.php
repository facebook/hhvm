<?hh // experimental

function foo(): void {
  try {
    throw new Exception("EXCEPTION");
  } catch (Exception e) {
    var_dump(e->getMessage());
  }
}

foo();
