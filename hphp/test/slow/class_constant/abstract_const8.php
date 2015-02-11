<?hh

interface I {
  abstract const foo;
}

class C implements I {
  const foo = 1;
}

// A specific HPHPC bug constant propagated the value of this 'foo' to the
// abstract constant declaration 'I::foo'
const foo = 123;

function main() {
  var_dump(foo);
  var_dump(C::foo);
  echo 'Done', "\n";
}

main();
