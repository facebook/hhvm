<?hh // strict

trait HelloTrait {
  public async function genHello(): Awaitable<string> {
    return 'Hello';
  }
}
class HelloClass {
  use HelloTrait {
    genHello as genHelloTrait;
  }

  <<__Override>>
  public async function genHello(): Awaitable<string> {
    $parent_str = await $this->genHelloTrait();
    return $parent_str.' World';
  }
}
