<?hh

async function gen_int(): Awaitable<int> {
  return 1;
}

class Klass {
  public async function foo(): Awaitable<int> {
    /*range-start*/
    concurrent {
        $x = await gen_int();
        $y = await gen_int();
    }
    /*range-end*/
    return $x + $y;
  }
}
