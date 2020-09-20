<?hh // strict

class GetClassTestBase {}

class GetClassTest extends GetClassTestBase {
  public static function test(): void {
    var_dump(get_parent_class());
    var_dump(get_parent_class(null));
    var_dump(get_parent_class(1));
    var_dump(get_parent_class(1.1));
    var_dump(get_parent_class("string"));
    var_dump(get_parent_class(GetClassTest::class));
    var_dump(get_parent_class(GetClassTestBase::class));
    var_dump(get_parent_class(new stdclass()));
  }
}

<<__EntryPoint>>
function main() {
  GetClassTest::test();
}
