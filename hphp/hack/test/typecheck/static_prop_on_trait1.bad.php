<?hh

trait MyTrait {
  public static vec<int> $ints = vec[];

  public static function addInt(int $i): void {
    MyTrait::$ints[] = $i;
  }
}

class C {
  use MyTrait;
}

<<__EntryPoint>>
function main(): void {
  C::addInt(42);
  \var_dump(C::$ints); // vec[] !!!
  \var_dump(MyTrait::$ints); // vec[42]
}
