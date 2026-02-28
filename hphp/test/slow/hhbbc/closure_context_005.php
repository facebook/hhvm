<?hh

class Foo {
  private $prop = vec[1,2,3];

  public function f() :mixed{
    return async function() {
      return function() {
        $this->prop = null;
      };
    };
  }

  public function getter() :mixed{ return $this->prop; }
}

function main() :mixed{
  $x = new Foo;
  var_dump($x->getter());
  $z = $x->f();
  $z = HH\Asio\join($z());
  var_dump($z);
  $z();
  var_dump($x->getter());
  var_dump($z);
  var_dump($x->getter());
}



<<__EntryPoint>>
function main_closure_context_005() :mixed{
main();
}
