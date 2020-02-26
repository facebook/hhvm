<?hh

class base {
  public function __call($x, $y) {
    echo "base::__call: $x, $y " . static::class . "\n";
  }
}

class one extends base {
}

class two extends base {
}

class goer {
  public function go($o) {
    $o->magic();
  }
}

function main() {
  $go = new goer;
  $one = new one;
  $two = new two;
  $go->go($one);
  foreach (varray[1,2,3] as $_) {
    foreach (varray[$one, $two] as $o) {
      $go->go($o);
      $go->go($o);
    }
  }
}
<<__EntryPoint>>
function main_entry(): void {

  // disable array -> "Array" conversion notice
  error_reporting(error_reporting() & ~E_NOTICE);

  main();
}
