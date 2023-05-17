<?hh

async function async_it(): AsyncIterator<string> {
  yield "key1";
  yield "key2";
}

class Klass {
    public static async function m(int $x): Awaitable<void> {
        $x = 0;
        /*range-start*/
        foreach (async_it() await as $v) {
          $x++;
        }
        /*range-end*/
        $y = $x;
    }
}

<<__EntryPoint>>
function main(): void {
   \HH\Asio\join(Klass::m(1));
}
