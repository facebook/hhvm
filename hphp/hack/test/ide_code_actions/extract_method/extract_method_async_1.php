<?hh

async function foo(): Awaitable<void> {}

class Klass {
  public async function foo(): Awaitable<void> {
    $ignore1 = 1;
    /*range-start*/
    $local = await foo();
    /*range-end*/
    $ignore2 = 1;
  }
}
