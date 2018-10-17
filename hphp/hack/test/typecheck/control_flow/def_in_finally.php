<?hh //strict

function f(): int {
  try {
  } finally {
    $id = 7;
  }
  return $id;
}
