<?hh

function h(): ?array {
  throw new Exception('');
}

function f(): void {
  $a = h();
  if (isset($a['a']) && true) {
  }
}
