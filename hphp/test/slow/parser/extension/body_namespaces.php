<?hh

namespace {
  class SomethingElse {}
}

namespace N1 {
  trait T1 { public function ignoreme1() {} }

  interface I1 { public function ignoreme2(); }
  interface I2 { public function ignoreme3(); }
  interface I3 extends I1, I2 { public function ignoreme4(); }

  class C1 { public function ignoreme5() {} }

  abstract class C2 { public function ignoreme6() {} }
  abstract final class C3 extends C2 {
    public static function yes_officer_this_method_here1() {}
  }

  class Something {}
  interface IDontcare {}
  interface IReallyDont {}
  class C4 extends Something implements IDontcare, IReallyDont {
    public function yes_officer_this_method_here2() {}
  }

  class C5 extends \SomethingElse {
    public function yes_officer_this_method_here3() {}
  }
}

namespace N2\N3 {
  class CompletelyDifferent {}
  class C6 extends CompletelyDifferent {
    public function yes_officer_this_method_here4() {}
  }
}

namespace {
  trait T2 { public function ignoreme7() {} }

  interface I4 { public function ignoreme8(); }
  interface I5 { public function ignoreme9(); }
  interface I6 extends I4, I5 { public function ignoreme10(); }

  class C7 { public function ignoreme11() {} }
  abstract class C8 { public function ignoreme12() {} }
  abstract final class C9 extends C8 {
    public static function yes_officer_this_method_here5() {}
  }

  class AnotherThing {}
  interface IDontcare {}
  interface IReallyDont {}
  class C10 extends AnotherThing implements IDontcare, IReallyDont {
    public function yes_officer_this_method_here6() {}
  }

  <<__EntryPoint>>
  function main() {
    $program = file_get_contents(__FILE__);
    $json = HH\ffp_parse_string($program);
    $results = HH\ExperimentalParserUtils\find_test_methods($json);
    \var_dump($results);
  }
}
