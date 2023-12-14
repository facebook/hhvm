<?hh

class A extends Exception {}
class B extends Exception {}
class C extends B {}

function blah() :mixed{
  echo "1\n";
  try {
    echo "2\n";
    try {
      echo "3\n";
      throw new Exception("ble!");
      echo "aaa\n";
    } catch (Exception $e) {
      echo "4\n";
    } finally {
      echo "5\n";
    }
    echo "6\n";
  } finally {
    echo "7\n";
    try {
      echo "8\n";
      try {
        echo "9\n";
        throw new A("bla!");
        echo "aaa\n";
      } finally {
        echo "10\n";
        throw new C("ble!");
        echo "bbb\n";
      }
      echo "ccc\n";
    } catch (A $ae) {
      exit("ddd\n");
    } finally {
      echo "11\n";
      try {
        echo "12\n";
        try {
          echo "13\n";
          throw new B("16");
          echo "fff\n";
        } finally {
          echo "14\n";
        }
        echo "ggg\n";
      } finally {
        echo "15\n";
      }
      echo "hhh\n";
    }
    echo "iii\n";
  }
  echo "jjj\n";
  return 666;
}
<<__EntryPoint>> function main(): void {
var_dump(blah());
}
