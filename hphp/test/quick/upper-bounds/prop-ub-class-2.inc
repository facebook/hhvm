class Foo <T1 as num, T2 as BarBar, T3 as T1, T4 as NoCheck> {
  public T1 $x;
  public ?T2 $y;
  static public ?T1 $sx;
  static public T2 $sy;
  <<__Soft>>
  public T1 $z;
  <<__LateInit>> public T2 $w;
  public T3 $v = 'd'; // not enforced
  public T4 $u = 10;
}

class Foo1 <T1 as num, T2 as BarBar, T3 as T1, T4 as NoCheck> {
  public T1 $x = null;
}
class Foo2 <T1 as num, T2 as BarBar, T3 as T1, T4 as NoCheck> {
  public ?T2 $y = 'a';
}
class Foo4 <T1 as num, T2 as BarBar, T3 as T1, T4 as NoCheck> {
  static public ?T1 $sx = 'b';
}
class Foo5 <T1 as num, T2 as BarBar, T3 as T1, T4 as NoCheck> {
  static public T2 $sy = null;
}
class Foo3 <T1 as num, T2 as BarBar, T3 as T1, T4 as NoCheck> {
  <<__Soft>>
  public T1 $z = 'c';
}
