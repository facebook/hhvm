<?hh // strict

class C<Ta, Tb> {
  public function f<Tc>(Ta $a, Tc $c): (Ta, Tc) {
    return tuple($a, $c);
  }

  public function g<Td>(Tb $b, Td $d): (Tb, Td) {
    return tuple($b, $d);
  }
}
