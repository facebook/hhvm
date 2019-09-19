<?hh

function using_($cgen) {
  foreach ($cgen() as $x) {
    yield $x;
  }
}

function broke() {
    foreach (using_(function() { yield 1; yield 2; yield 3; }) as $x) {
      var_dump($x);
    }
}

class c {
  function genclo() {
    return function() {
      yield $this;
    };
  }
}

function main() {
  $c = new c;
  $f = $c->genclo();
  foreach ($f() as $v) {
    var_dump($v);
  }
}
<<__EntryPoint>>
function main_entry(): void {
  broke();
  main();
}
