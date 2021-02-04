<?hh

class C {
  <<__Soft>> public string $s = "four";
}

function f(<<__Soft>> int $i): <<__Soft>> float {
  hh_show((new C())->s);
  hh_show($i);
  hh_show(f(3));
  return 3.14;
}
