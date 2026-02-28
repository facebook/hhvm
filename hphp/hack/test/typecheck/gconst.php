//// gconst.hhi
<?hh

const int X1 = 0;
const int X2;
const string Y;
const FunctionCredential Z;
const int BAD = 'hello';

//// gconst.php
<?hh

function test(
  int $i,
  string $s,
  FunctionCredential $fc,
): void {}

function main(): void {
  test(__LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__);

  test(X1, Y, Z);

  test(X1, X2, Z);
}
