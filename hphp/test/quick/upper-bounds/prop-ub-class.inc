interface Barable {}
class Bar implements Barable {}
class NoBar {}

type BarBar = Barable;

class Foo <T1 as num, T2 as BarBar> {
  public T1 $x = null;
  public ?T2 $y = 'a';
  static public ?T1 $sx = 'b';
  static public T2 $sy = null;
  public @T1 $z = 'c';
  <<__LateInit>> public T2 $w;
}

