<?hh

async function gen_void(): Awaitable<void> {}

async function async_keyed_it(): AsyncKeyedIterator<string, int> {
  yield "key1" => 1;
  yield "key2" => 2;
}

class Klass {
    public static async function m(int $x): Awaitable<void> {
        $x = 0;
        /*range-start*/
        foreach (async_keyed_it() await as $k => $v) {
          $x++
        }
        await gen_void();
        /*range-end*/
        $y = $x;
    }
}

<<__EntryPoint>>
function main(): void {
   \HH\Asio\join(Klass::m(1));
}
