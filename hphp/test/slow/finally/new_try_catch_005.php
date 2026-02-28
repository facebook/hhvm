<?hh

class A extends Exception {}
class B extends Exception {}
class C extends B {}

function blah() :mixed{
  echo "1\n";
  try {
    echo "2\n";
    throw new Exception("10");
    echo "aaa\n";
  } finally {
    echo "3\n";
    try {
      echo "4\n";
      try {
        echo "5\n";
        throw new A("bla!");
        echo "bbb\n";
      } finally {
        echo "6\n";
        throw new C("blu!");
        echo "ccc\n";
      }
      echo "ddd\n";
    } catch (A $ae) {
      echo("eee\n");
    } catch (B $ab) {
      echo "7\n";
    } catch (Exception $e) {
      exit("fff\n");
    } finally {
      echo "8\n";
    }
    echo "9\n";
  }
  echo "ggg\n";
  return 666;
}
<<__EntryPoint>> function main(): void {
var_dump(blah());
}
