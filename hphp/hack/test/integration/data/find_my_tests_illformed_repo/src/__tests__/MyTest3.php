<?hh

class MyTest3 extends WWWTest {

  public function testTarget(): void {
    // This one should be found: We depend on an ill-formed file, but also have the path via well-formed files only.
    IllFormedClass::foo();
    WellFormedClass::foo();
  }

}
