<?hh

function f(): void {
  true ?
    dict['a' => 42]
  :
    dict['b' => true];
}
