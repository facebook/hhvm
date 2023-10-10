<?hh

class GetClassTest {
  public static function test(): void {
    var_dump(get_class());
    var_dump(get_class(null));
    var_dump(get_class(1));
    var_dump(get_class(1.1));
    var_dump(get_class("string"));
    var_dump(get_class(new stdClass()));
  }
}

<<__EntryPoint>>
function main() :mixed{
  GetClassTest::test();
}
