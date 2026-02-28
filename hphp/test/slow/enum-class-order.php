<?hh

enum class ChildEnum: stdClass extends ParentEnum {
  stdClass Foo = new stdClass();
}

enum class ParentEnum: stdClass extends GrandParentEnum {
  stdClass Bar = new stdClass();
}

enum class GrandParentEnum: stdClass {
  stdClass Baz = new stdClass();
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(ChildEnum::Foo, ChildEnum::Bar, ChildEnum::Baz);
}
