<?hh

class Foo {
  const dict<string, int> FOO = dict[
    'a' => 1,
    'b' => 2,
  ];

  public function __construct(private string $type) {}

  public function get(): int {
    return self::FOO[$this->type];
  }
}

<<__EntryPoint>>
function main(): void {
  $a = new Foo('a');
  var_dump($a->get());
}
