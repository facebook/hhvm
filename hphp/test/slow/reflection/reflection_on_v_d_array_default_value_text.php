<?hh

class C {
    const A = 1;
}

function f(
  $p0 = varray[C::A, 1],
  $p1 = varray[2, 1, 3],
  $p2 = varray[1, 2, 3]
  ) {}

function g(
  $p0 = darray[1 => C::A, 2 => 1],
  $p1 = darray[1 => 2, 4 => 1, 3 => 3],
  $p2 = darray[1 => 1, "hi" => 2, 5 => 3]
  ) {}

$f = new ReflectionFunction("f");
$p0 = $f->getParameters()[0]->getDefaultValueText();
$p1 = $f->getParameters()[1]->getDefaultValueText();
$p2 = $f->getParameters()[2]->getDefaultValueText();
var_dump($p0, $p1, $p2);

$g = new ReflectionFunction("g");
$p0 = $g->getParameters()[0]->getDefaultValueText();
$p1 = $g->getParameters()[1]->getDefaultValueText();
$p2 = $g->getParameters()[2]->getDefaultValueText();
var_dump($p0, $p1, $p2);

