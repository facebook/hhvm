<?hh
class TestStringEnum extends Enum<string> {
  const A = 'A';
}
class TestIntEnum extends Enum<int> {
  const ONE = 1;
}
class TestStringApiEnum extends ApiEnum<string> {
  const A = 'A';
}
class TestIntApiEnum extends ApiEnum<int> {
  const ONE = 1;
}
class TestRootClass {
}
class TestSubclassingOtherClass extends TestRootClass {
}
enum TestModernEnumString: string {
  A = 'A';
}
enum TestModernEnumInt: int {
  ONE = 1;
}
