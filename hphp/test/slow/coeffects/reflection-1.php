<?hh

function f()[] {}
function g()[rx, write_props, lol, read_globals, globals] {}
function h() {}

class Something {}
class C<reify Tc1> {
  public function f<Tf1, reify Tf2>(
    Something $x1,
    (function()[_]: void) $x2,
  )[$x1::C, ctx $x2, this::C, IO, this::T1::T2::C2, Tc1::T1::T2::C, Tf2::C] {}
}

<<__EntryPoint>>
function main() {
  var_dump((new ReflectionFunction('f'))->getCoeffects());
  var_dump((new ReflectionFunction('g'))->getCoeffects());
  var_dump((new ReflectionFunction('h'))->getCoeffects());
  var_dump((new ReflectionMethod('C', 'f'))->getCoeffects());
}
