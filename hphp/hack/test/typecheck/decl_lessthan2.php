<?hh

function inTheFuture(DateTime $x): bool {
  if ($x < new DateTime()) {
    return false;
  } else {
    return true;
  }
}
