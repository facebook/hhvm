<?hh

class BadClass extends DoesntExist {
  public static function bar() {
    return $x ==> $x + 1;
  }
}

<<__EntryPoint>>
function main() {
  echo "FAIL\n";
}
