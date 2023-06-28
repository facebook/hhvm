<?hh

class F {
  const async = 'async';
  static async function foo () :Awaitable<mixed>{
    return self::async;
  }
}




<<__EntryPoint>>
function main_const_async() :mixed{
$async = new F;
var_dump(F::async);
var_dump($async::async);
var_dump(HH\Asio\join(F::foo()));
}
