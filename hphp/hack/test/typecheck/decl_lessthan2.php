<?hh // decl

function inTheFuture($x) {
  if ($x < new DateTime()) {
    return false;
  }
  else {
    return true;
  }

}
