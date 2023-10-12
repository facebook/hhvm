<?hh

function f(): void {
  $y = &$$s + 1;
}

function f1(): void {
  $y = &$$$$$s + 1;
}
