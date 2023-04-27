<?hh

class C {}

function generator($n) {
  while (--$n > 0) {
    yield new C();
  }
}

<<__EntryPoint>>
function main() {
  ini_set('memory_limit', 30000);

  foreach (generator(100000) as $_) {
  }

  echo "OK\n";
}
