<?hh

class Ref {
  public function __construct(public $val)[] {}
}

class Foo { const Bar = 12; }
trait Yoyo {
  function cl($k) :mixed{
    $ref = new Ref(42);
    return function() use ($k, $ref) {
      yield $ref->val++ . "2\n";
      yield $ref->val++ . "2\n";
      yield $ref->val++ . "2\n";
    };
  }
}

class Blah {
  use Yoyo;
}

function main() :mixed{
  $k = (new Blah)->cl(1);
  foreach ($k() as $x) {
    echo $x . "\n";
  }
  foreach ($k() as $x) {
    echo $x . "\n";
  }
  $k = (new Blah)->cl(1);
  foreach ($k() as $x) {
    echo $x . "\n";
  }
}

<<__EntryPoint>>
function main_static_closure_trait_generator() :mixed{
;

main();
}
