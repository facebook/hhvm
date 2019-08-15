<?hh

class A {
  public static dict<int, string> $sa = dict[2 => "green", 10 => "home"];
  <<__Const>>
  public static dict<int, string> $csa = dict[2 => "green", 10 => "home"];
  <<__Const>>
  public dict<int, string> $ca = dict[2 => "green", 10 => "home"];
  <<__Const>>
  public static dict<int, dict<int, string>> $csa2 =
    dict[2 => dict[21 => "a", 22 => "b"], 3 => dict[33 => "c", 34 => "d"]];
  <<__Const>>
  public static int $index = 2;


  public function write_const_static(): void{
    unset(static::$csa[2]);
    unset($this->ca[2]);
    unset(static::$sa[2]); // ok
    unset(static::$sa[static::$index]); //ok
  }

}

function test(): void {
  unset(A::$sa[2]);        // ok
  unset(A::$csa[2]);
  unset(A::$csa[2], A::$csa[2]);
  unset(A::$csa2[2][21]);
}

function f<reify T as A>(): void {
  unset(T::$sa[2]);        // ok
  unset(T::$csa[2]);
}
