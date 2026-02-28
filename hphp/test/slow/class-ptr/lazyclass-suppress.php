<?hh

<<__EntryPoint>>
function main() :mixed{
  require 'lazyclass-suppress.inc';

  // type_structure Methods
  var_dump(type_structure(FooTransparentAlias::class));
  var_dump(type_structure(FooOpaqueAlias::class));
  var_dump(type_structure(Baz::class, 'T'));
  var_dump(type_structure(Bang::class, 'U'));
  var_dump(\HH\type_structure_classname(FooTransparentAlias::class));
  var_dump(\HH\type_structure_classname(FooOpaqueAlias::class));
  var_dump(\HH\type_structure_classname(Baz::class, 'T'));
  var_dump(\HH\type_structure_classname(Bang::class, 'U'));

  // Thrift Serialization Methods
  $p = new DummyProtocol();
  $foo = Foo::class;
  var_dump($foo);
  $foo_name = "" . $foo;
  var_dump($foo_name);

  $v = new Foo();
  $v->nested = new NestedStruct("foo bar baz bang");
  thrift_protocol_write_binary($p, 'FooMethod', 1, $v, 20, true);
  var_dump(thrift_protocol_read_binary($p, 'Foo', true));

  $v = new Foo();
  $v->nested = new NestedStruct("bang baz bar foo");
  thrift_protocol_write_compact2($p, 'FooMethod', 2, $v, 20);
  var_dump(thrift_protocol_read_compact($p, 'Foo'));

  $foo = Foo::class;
  var_dump($foo);
  $foo_name = "" . $foo;
  var_dump($foo_name);
}
