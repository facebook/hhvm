<?hh

function afunction(): int {
  return bfunction();
}

function bfunction(): int {
  return 42;
}
