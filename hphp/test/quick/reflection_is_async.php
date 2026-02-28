<?hh

function foo () :mixed{}
async function async_foo() :Awaitable<mixed>{}

class Bar {
  public function foo () :mixed{}
  public async function asyncFoo() :Awaitable<mixed>{}
}
<<__EntryPoint>> function main(): void {
var_dump((new ReflectionFunction('foo'))->isAsync());
var_dump((new ReflectionFunction('async_foo'))->isAsync());
var_dump((new ReflectionMethod('Bar::foo'))->isAsync());
var_dump((new ReflectionMethod('Bar::asyncFoo'))->isAsync());
var_dump((new ReflectionFunction(async function () {}))->isAsync());
}
