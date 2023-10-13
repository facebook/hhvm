<?hh

class C {
  public function f<Ta>(Ta $a): Ta {
    return $a;
  }

  public function g<Tb>(Tb $b): Tb {
    return $b;
  }
}
