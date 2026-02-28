<?hh

class :foo {
  attribute string name;

  final public function __construct(KeyedTraversable<string, mixed> $attributes,
                                    Traversable<XHPChild> $_children) {
    var_dump(get_class($this));
    var_dump($attributes);
  }
}

class :bar extends :foo {
  attribute string name, int age;
}


<<__EntryPoint>>
function main_xhp_spread() :mixed{
$bar = <bar name="test" age={21} />;
<foo {...$bar} />;
<foo {...$bar} name="foo2" />;
<foo {...$bar} name="foo2" {...$bar} />;
<foo {...<bar name="test" age={21} />} />;
<foo {...<bar name="test" {...$bar} age={21} />} />;
<foo {...<bar />} />;
}
