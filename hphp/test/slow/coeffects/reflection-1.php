<?hh

function f()[] :mixed{}
function g()[rx, write_props, lol, read_globals, globals] :mixed{}
function h() :mixed{}

class Something {}
class C<reify Tc1> {
  public function f<Tf1, reify Tf2>(
    Something $x1,
    (function()[_]: void) $x2,
  )[$x1::C, ctx $x2, this::C, IO, this::T1::T2::C2, Tc1::T1::T2::C, Tf2::C] :mixed{}
}

<<__EntryPoint>>
function main() :mixed{
  var_dump((new ReflectionFunction('f'))->getCoeffects());
  $result = (new ReflectionFunction('g'))->getCoeffects();
  sort(inout $result);
  var_dump($result);
  var_dump((new ReflectionFunction('h'))->getCoeffects());
  var_dump((new ReflectionMethod('C', 'f'))->getCoeffects());
}
