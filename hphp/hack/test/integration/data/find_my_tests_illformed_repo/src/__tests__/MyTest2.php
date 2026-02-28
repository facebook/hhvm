<?hh

class MyTest2 extends WWWTest {

  public function testTarget(): void {
    // Not expecting this one to be found, IllFormedClass has a syntax error
    // (even though the method called below is fine).
    IllFormedClass::foo();
  }

}
