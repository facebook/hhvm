//// cconst.hhi
<?hh

class Klass {
  const int X1 = 0;
  const int X2;
  const string Y;
  const int BAD = 'hello';
}

//// cconst.php
<?hh

function test(
  int $i,
  string $s,
): void {}

function main(): void {
  test(\ReflectionMethod::IS_PUBLIC, \DateTime::ISO8601);

  test(Klass::X1, Klass::Y);

  test(Klass::X1, Klass::X2);
}
