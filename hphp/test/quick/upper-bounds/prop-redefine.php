<?hh

class Sup <T1 as num> {
  public T1 $x;
}

class Sub <T2 as int> extends Sup {
  public T2 $x;
}

<<__EntryPoint>> function main() :mixed{
  $o = new Sub;
  $o->x = 10;
  var_dump($o->x);
}
