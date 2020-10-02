<?hh // partial

function h(): ?varray {
  throw new Exception('');
}

function f(): void {
  $a = h();
  if (isset($a['a']) && true) {
  }
}
