<?hh
class Foo {
  const BONK = "ok 1\n";
  const BEEP BOOP = "ok 2\n";
  const a<b<c,d>,e> BLEH = "ok 3\n";
}
const BONK = "ok 4\n";
const BEEP BOOP = "ok 5\n";
const a<b<c,d>,e> BLEH = "ok 6\n";

echo "1..6\n", Foo::BONK, Foo::BOOP, Foo::BLEH, BONK, BOOP, BLEH;
