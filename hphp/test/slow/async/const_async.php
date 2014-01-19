<?hh

class F {
  const async = 'async';
  async function foo () {
    return self::async;
  }
}

$async = new F;
var_dump(F::async);
var_dump($async::async);
var_dump(F::foo()->join());


