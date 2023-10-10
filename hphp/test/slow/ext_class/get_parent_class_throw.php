<?hh

class GetClassTestBase {}

class GetClassTest extends GetClassTestBase {
  public static function test(): void {
    try { var_dump(get_parent_class()); } catch (Exception $e) { var_dump($e->getMessage()); }
    try { var_dump(get_parent_class(null)); } catch (Exception $e) { var_dump($e->getMessage()); }
    try { var_dump(get_parent_class(1)); } catch (Exception $e) { var_dump($e->getMessage()); }
    try { var_dump(get_parent_class(1.1)); } catch (Exception $e) { var_dump($e->getMessage()); }
    try { var_dump(get_parent_class("string")); } catch (Exception $e) { var_dump($e->getMessage()); }
    var_dump(get_parent_class(GetClassTest::class));
    var_dump(get_parent_class(GetClassTestBase::class));
    var_dump(get_parent_class(new stdClass()));
  }
}

<<__EntryPoint>>
function main() :mixed{
  GetClassTest::test();
}
