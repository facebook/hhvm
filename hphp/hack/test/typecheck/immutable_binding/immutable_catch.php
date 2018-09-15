<?hh // experimental

function foo(): void {
  try {
    throw new Exception();
  } catch (Exception e) {
    echo e->getMessage();
  }
}
