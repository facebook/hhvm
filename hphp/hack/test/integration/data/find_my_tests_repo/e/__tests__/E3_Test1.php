<?hh

// TODO(T246671457)
// This test is currently not selected when modifying E3_Super, even though it should!

class E3_Test1 extends WWWTest {

  public static function test(): void {
    E3_Helper::helper1();
  }
}
