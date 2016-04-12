<?hh

function f() {
  invariant(1 == 1, 'test');
  invariant_violation('test2');
}
