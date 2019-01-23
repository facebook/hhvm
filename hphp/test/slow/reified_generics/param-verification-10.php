<?hh

class A<reify Ta, reify Tb> {}

class C<reify Ta> {
  function f<reify Tb>(A<Ta, Tb> $_) {}
}

$c = new C<reify int>();
$c->f<reify string>(new A<reify int, reify string>());
$c->f<reify int>(new A<reify int, reify string>());
