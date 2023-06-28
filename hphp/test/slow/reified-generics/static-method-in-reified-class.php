<?hh

class C<reify T> {
  private static function f() :mixed{}
}

// As long as it does not use T in f it is fine
<<__EntryPoint>> function main(): void {
echo "done\n";
}
