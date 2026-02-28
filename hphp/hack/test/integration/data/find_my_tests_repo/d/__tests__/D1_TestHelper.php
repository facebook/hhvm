<?hh

// Note that this is not a WWWTest!
//
// This is here to test the case where we use D1::class, but not from a test file itself
// (which is boring, because then the test gets selected anyway)
class D1_TestHelper {

  public static function getClass(): class<D1> {
    return D1::class;
  }

}
