<?hh

function f(): void {
  if (true) {
    dict['a' => 42];
  } else {
    dict['b' => true];
  }
}
