<?hh

interface IHerp {
  const FOO = 'bar';
}

class Derp implements IHerp {
  const FOO = 'baz';
}
<<__EntryPoint>> function main(): void {
var_dump('All classes declared');
}
