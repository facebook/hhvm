<?hh

final class X {
  private function foo() :mixed{
    $r = darray[1 => varray[]];
    $r[1][] = 55;
    $r[1][] = "foo";

    return $r;
  }

  public function bar() :mixed{
    $s = $this->foo();

    foreach ($s as $v) {
      if (!$v) {
        continue;
      }

      var_dump($v);
    }
  }
}


<<__EntryPoint>>
function main_assert_ratl() :mixed{
(new X)->bar();
}
