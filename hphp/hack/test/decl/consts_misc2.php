<?hh

final abstract class TestC {
  public static function i(int $x)[write_props]: int {return $x;}
  const int i = 3;
}
enum TestE: int as int {X = 4; Y = 5;}

final abstract class TestRefs1 {
  const mixed A = shape(TestE::X => 1);  // HHVM evaluates [TestE::X]
  const mixed B = shape(TestE::X => TestE::Y, TestE::Y => 6);  // HHVM evaluates [TestE::X, TestE::Y]
  const mixed C = +TestE::X;  // HHVM evaluates [TestE::X]
  const mixed D = true ? TestE::X : 7;  // HHVM evaluates [TestE::X]
  const mixed E = TestC::class;  // HHVM evaluates []
}

enum class TestRefs2:mixed {
  string K1 = "b";
  string K2 = "c";
  mixed B = shape(TestRefs2::K1 => 1, self::K2 => 2); // HHVM evaluates [TestRefs2::K1, self::K2]
  mixed F = TestC::i(4);  // HHVM evaluates [TestC::i], the function
  mixed G = TestC::i(TestC::i);  // HHVM evaluates [TestC::i, TestC::i] both function and constant
  mixed H = TestC::i<>;  // HHVM evaluates []
}
