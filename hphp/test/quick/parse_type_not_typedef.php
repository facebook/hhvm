<?hh

function type() {  // ok, 'type' is context sensitive
  echo "Hi\n";
}
type();

class Foo {
  const TYPE = 'hi2';
}

echo Foo::TYPE . "\n";

type t = int;
function wat(t $type) {
  echo $type . "\n";
}
wat(12);
