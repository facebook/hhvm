<?hh

enum class ChildEnum: stdclass extends ParentEnum {
  stdclass Foo = new stdclass();
}

enum class ParentEnum: stdclass extends GrandParentEnum {
  stdclass Bar = new stdclass();
}

enum class GrandParentEnum: stdclass {
  stdclass Baz = new stdclass();
}

<<__EntryPoint>>
function main() {
  var_dump(ChildEnum::Foo, ChildEnum::Bar, ChildEnum::Baz);
}
