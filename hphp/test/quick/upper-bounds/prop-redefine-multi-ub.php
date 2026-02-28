<?hh
interface Fooable {}
interface Barable {}

class Sup <T as Fooable as Barable> {
  <<__LateInit>> public T $x;
}

class Sub1 <T as Barable as Fooable> extends Sup {
  <<__LateInit>> public T $x;
}

class Sub2 <T as Fooable as string> extends Sup {
  <<__LateInit>> public T $x;
}

<<__EntryPoint>> function main() :mixed{
  var_dump(new Sub1);
  var_dump(new Sub2);
}
