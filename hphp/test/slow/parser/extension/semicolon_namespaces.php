<?hh

namespace N1;

trait T1 { public function ignoreme1() :mixed{} }

interface I1 { public function ignoreme2():mixed; }
interface I2 { public function ignoreme3():mixed; }
interface I3 extends I1, I2 { public function ignoreme4():mixed; }

class C1 { public function ignoreme5() :mixed{} }

abstract class C2 { public function ignoreme6() :mixed{} }
abstract final class C3 extends C2 {
  public static function yes_officer_this_method_here1() :mixed{}
}

class Something {}
interface IDontcare {}
interface IReallyDont {}
class C4 extends Something implements IDontcare, IReallyDont {
  public function yes_officer_this_method_here2() :mixed{}
}

class SomethingElse {}
class C5 extends \N1\SomethingElse {
  public function yes_officer_this_method_here3() :mixed{}
}

namespace N2\N3;

trait T2 { public function ignoreme7() :mixed{} }

interface I4 { public function ignoreme8():mixed; }
interface I5 { public function ignoreme9():mixed; }
interface I6 extends I4, I5 { public function ignoreme10():mixed; }

class C6 { public function ignoreme11() :mixed{} }
abstract class C7 { public function ignoreme12() :mixed{} }
abstract final class C8 extends C7 {
  public static function yes_officer_this_method_here4() :mixed{}
}

class AnotherThing {}
interface IDontcare {}
interface IReallyDont {}
class C9 extends AnotherThing implements IDontcare, IReallyDont {
  public function yes_officer_this_method_here5() :mixed{}
}

<<__EntryPoint>>
function main() :mixed{
  $program = \file_get_contents(__FILE__);
  $json = \HH\ffp_parse_string($program);
  $results = \HH\ExperimentalParserUtils\find_test_methods($json);
  \var_dump($results);
}
