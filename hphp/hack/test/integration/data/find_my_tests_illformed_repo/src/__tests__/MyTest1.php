<?hh

class MyTest1 extends WWWTest {

  public function testTarget(): void {
    // This one should be found: All files on the path are well-formed.
    WellFormedClass::foo();
  }

}
