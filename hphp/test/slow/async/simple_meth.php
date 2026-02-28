<?hh

class F {
  function __construct($a) { $this->a = $a; }

  async function retA() :Awaitable<mixed>{ return $this->a; }
  async function awaitA() :Awaitable<mixed>{
    $b = await $this->retA();
    return 1 + $b;
  }
}


<<__EntryPoint>>
function main_simple_meth() :mixed{
$f = new F(42);
var_dump(HH\Asio\join($f->retA()));
var_dump(HH\Asio\join($f->awaitA()));
}
