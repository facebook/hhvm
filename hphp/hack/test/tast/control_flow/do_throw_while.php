<?hh

function throwFromDoWhileLoop(bool $cond): void {
  do {
    throw new Exception("DoWhileBody");
  } while ($cond);
}
