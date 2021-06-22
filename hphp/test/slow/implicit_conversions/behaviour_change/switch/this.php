<?hh

<<__EntryPoint>>
function main() {
  $x = new X();
  $x->bar(new stdClass);
  $x->bar($x);
  $x->bar(new X());
}

class X {
  function bar($arg) {
    switch ($this) {
      case $arg:
        echo "arg\n";
        break;
      default:
        echo "def\n";
    }
    switch ($arg) {
      case $this:
        echo "this\n";
        break;
      default:
        echo "def\n";
    }
  }
}
