<?hh // strict

class GetClassTest {
  public static function test(): void {
    try { var_dump(get_class()); } catch (Exception $e) { var_dump($e->getMessage()); }
    try { var_dump(get_class(null)); } catch (Exception $e) { var_dump($e->getMessage()); }
    try { var_dump(get_class(1)); } catch (Exception $e) { var_dump($e->getMessage()); }
    try { var_dump(get_class(1.1)); } catch (Exception $e) { var_dump($e->getMessage()); }
    try { var_dump(get_class("string")); } catch (Exception $e) { var_dump($e->getMessage()); }
    var_dump(get_class(new stdclass()));
  }
}

<<__EntryPoint>>
function main() {
  GetClassTest::test();
}
