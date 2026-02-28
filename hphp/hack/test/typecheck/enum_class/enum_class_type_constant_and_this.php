<?hh

interface AnimalTest {}

interface AnimalTestRunner {}

abstract enum class AnimalTestRunners: AnimalTestRunner {

  abstract const type TTest as AnimalTest;
}

final class FastAnimalTestRunner<T as AnimalTest> implements AnimalTestRunner {
  public function runTest(T $_test): void {}
}

final class DogTest implements AnimalTest {

  public function run<T as FastAnimalTestRunner<this>>(
    HH\EnumClass\Label<FastAnimalTestRunners, T> $label,
  ): void {
    FastAnimalTestRunners::valueOf($label)->runTest($this);
  }
}

enum class FastAnimalTestRunners: AnimalTestRunner extends AnimalTestRunners {

  const type TTest = DogTest;

  FastAnimalTestRunner<this::TTest> FastDog = new FastAnimalTestRunner();
  FastAnimalTestRunner<DogTest> FastDog2 = new FastAnimalTestRunner();
}

final class MyEnumClassTest {

  const type TTest = int;

  public function baz(DogTest $test): void {
    $test->run(#FastDog); // Hack error: Expected int but got DogTest
    $test->run(#FastDog2); // No Hack error
  }
}

final class MyEnumClassTest2 {

  const type TTest2 = int;

  public function baz(DogTest $test): void {
    $test->run(#FastDog); // No Hack error
    $test->run(#FastDog2); // No Hack error
  }
}

final class MyEnumClassTest3 {

  const type TTest = DogTest;

  public function baz(DogTest $test): void {
    $test->run(#FastDog); // No Hack error
    $test->run(#FastDog2); // No Hack error
  }
}
