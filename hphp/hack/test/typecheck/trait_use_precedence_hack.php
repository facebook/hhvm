<?hh // strict

trait GoodbyeTrait {
  public async function genHello(): Awaitable<string> {
    return 'Goodbye';
  }
}
trait HelloTrait {
  public async function genHello(): Awaitable<string> {
    return 'Hello';
  }
}
class HelloClass {
  use HelloTrait, GoodbyeTrait {
    HelloTrait::genHello insteadof GoodbyeTrait;
  }

  <<__Override>>
  public async function genHello(): Awaitable<string> {
    $parent_str = await $this->genHelloTrait();
    return $parent_str.' World';
  }
}
