<?hh // strict

class C implements Awaitable<this> {
  public static async function gen(): Awaitable<this> {
    // $x is dependent type `static as C
    $x = self::get();
    hh_show($x);
    // Awaiting $x should return 'this' which is dependent type `static as C
    $x = await $x;
    hh_show($x);
    return $x;
  }

  public static function get(): this {
    // UNSAFE
  }

  public function getWaitHandle(): WaitHandle<this> {
    // UNSAFE
  }
}
