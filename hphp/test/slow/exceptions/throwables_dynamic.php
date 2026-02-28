<?hh

class NonThrowable {}

function raise($what) :mixed{
  try {
    throw $what;
  } catch (Throwable $t) {
    echo "caught[good]: ".get_class($t)."\n";
  } catch (NonThrowable $nt) {
    echo "caught[bad]: ".get_class($nt)."\n";
  }
}

<<__EntryPoint>>
function main() :mixed{
  raise(new Exception());
  raise(new InvalidOperationException());
  raise(new Error());
  raise(new AssertionError());
  raise(new NonThrowable());
}
