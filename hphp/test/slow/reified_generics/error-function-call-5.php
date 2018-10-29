<?hh

class C {
  function f<reified Ta, reified Tb>() {}
}

$c = new C();
$c->f<reified string>();
