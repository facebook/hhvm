class Foo <T1 as num, T2 as BarBar, T3 as T1, T4 as NoCheck> {
  public T1 $x = null;
  public ?T2 $y = 'a';
  static public ?T1 $sx = 'b';
  static public T2 $sy = null;
  <<__Soft>>
  public T1 $z = 'c';
  <<__LateInit>> public T2 $w;
  public T3 $v = 'd'; // not enforced
  public T4 $u = 10;
}

