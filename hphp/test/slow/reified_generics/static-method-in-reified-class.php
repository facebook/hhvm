<?hh

class C<reify T> {
  private static function f() {}
}

// As long as it does not use T in f it is fine
echo "done\n";
