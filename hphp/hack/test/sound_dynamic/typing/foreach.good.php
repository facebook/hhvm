<?hh

class C {
  public function f(dynamic $d) : dynamic {
    foreach ($d as $k => $v) {
      if ($d) {
        return $v->m();
      }
      else {
        return $k->m();
      }
    }
    foreach ($d as $v) {
      return $v->m();
    }
    return $d;
  }
}
