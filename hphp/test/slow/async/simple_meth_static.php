<?hh

class F {
  static async function ret1() :Awaitable<mixed>{ return 1; }
  static async function await1() :Awaitable<mixed>{
    $b = await F::ret1();
    return 1 + $b;
  }
}


<<__EntryPoint>>
function main_simple_meth_static() :mixed{
var_dump(HH\Asio\join(F::ret1()));
var_dump(HH\Asio\join(F::await1()));
}
