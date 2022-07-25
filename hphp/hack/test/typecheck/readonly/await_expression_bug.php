<?hh

class T {
  public int $prop = 0;
  public async function set(): Awaitable<int> {

    $this->prop = 5;
    return 5;
  }

}

<<__EntryPoint>>
  async function test(): Awaitable<void> {
  $y = readonly new T();
  $z = (await $y->set()) + 5;
}
