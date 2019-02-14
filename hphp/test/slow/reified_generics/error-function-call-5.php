<?hh

class C {
  function f<reify Ta, reify Tb>() {}
}

$c = new C();
$c->f<string>();
