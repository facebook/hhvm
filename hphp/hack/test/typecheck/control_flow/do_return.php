<?hh //strict

function f(bool $b): int {
  do {
    return 0;
  } while ($b);
  return 1;
}
