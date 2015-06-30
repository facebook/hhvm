<?hh

function foo() {
  global $z;
  apc_store('x', array(Vector {1, 2, 3}));
  $z = apc_fetch('x');
  apc_store('x', null);
}

foo();
echo "ok\n";
