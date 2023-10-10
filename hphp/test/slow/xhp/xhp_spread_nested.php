<?hh

class :foo {
  attribute string name, int age;

  final public function __construct(KeyedTraversable<string, mixed> $attributes,
                                    Traversable<XHPChild> $_children) {
    var_dump(get_class($this));
    var_dump($attributes);
  }
}

function test() :mixed{
  <foo {
    ...<foo {
        ...<foo age={3} />
      } {
        ...<foo {...<foo age={1} />} age={2} />
      } age={4} />
  } age={5} />;
}

<<__EntryPoint>>
function main_xhp_spread_nested() :mixed{
test();
}
