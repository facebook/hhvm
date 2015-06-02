<?hh

class Heh { public function yo() { echo "yo\n"; } }

async function bar() { return new Heh(); }
async function foo() {
  $x = await bar();
  $x->yo();
  return $x;
}

function main() {
  $go = foo();
  $blah = HH\Asio\join($go);
  $blah->yo();
}

main();
