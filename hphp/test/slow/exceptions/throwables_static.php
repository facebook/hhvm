<?hh

class NonThrowable {}

<<__EntryPoint>>
function main() :mixed{
  try {
    throw new Exception();
  } catch (Throwable $t) {
    echo "caught[good]: ".get_class($t)."\n";
  }

  try {
    throw new InvalidOperationException();
  } catch (Throwable $t) {
    echo "caught[good]: ".get_class($t)."\n";
  }

  try {
    throw new Error();
  } catch (Throwable $t) {
    echo "caught[good]: ".get_class($t)."\n";
  }

  try {
    throw new AssertionError();
  } catch (Throwable $t) {
    echo "caught[good]: ".get_class($t)."\n";
  }

  try {
    throw new NonThrowable();
  } catch (NonThrowable $nt) {
    echo "caught[bad]: ".get_class($nt)."\n";
  }
}
