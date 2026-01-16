<?hh

class B {
  public HH\Map<string, float> $fields;
  public function __construct() {
    $this->fields = HH\Map<string, float>{"foo".HH\Lib\PseudoRandom\int(0, 100)  => 1.1};
  }
}

class A {
  public static async function bar() : Awaitable<bool> {
    $results = await A::baz();
    return $results === null ? false :  $results->foo > 1.0;
  }

  public static async function baz()
    : Awaitable<?dict<string, float>> {
    var_dump("Invoked userAuto");
    return await A::wrapAsync(async {
      $raw = await A::genBuzz();
      return $raw->fields;
    });
  }

  public static async function wrapAsync<T>(Awaitable<T> $awaitable) : Awaitable<T> {
      return await $awaitable;
  }

  public static async function genBuzz() : Awaitable<dynamic> {
    await HH\Asio\usleep(10 * 1000);
    return new B();
  }
}

<<__EntryPoint>>
async function main() : Awaitable<void> {
  var_dump(await A::bar());
}
