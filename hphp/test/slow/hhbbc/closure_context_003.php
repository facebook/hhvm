<?hh

class Foo {
  private $prop = vec[1,2,3];

  public async function genFoo() :Awaitable<mixed>{
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
  $z = HH\Asio\join($x->genFoo());
  $z = HH\Asio\join($z());
  var_dump($z);
  $z();
  var_dump($x->getter());
  var_dump($z);
  var_dump($x->getter());
}



<<__EntryPoint>>
function main_closure_context_003() :mixed{
main();
}
