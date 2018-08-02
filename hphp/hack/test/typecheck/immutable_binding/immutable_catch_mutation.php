<?hh // experimental

function foo(): void {
  try {
    throw new Exception();
  } catch (Exception e) {
    e = new Exception();
    throw e;
  }
}
