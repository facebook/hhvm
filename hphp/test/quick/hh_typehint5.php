<?hh

class Bar {}
function foo(?Bar $x) {
  echo "unreached\n";
}
foo(); // KindOfUninit should not pass type hint.
