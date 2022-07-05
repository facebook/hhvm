<?hh

type ExceptionAlias = Exception;

<<__EntryPoint>>
function my_main(): void {
  try {
    throw new Exception();
  } catch (ExceptionAlias $a) {
    echo "never executed";
  }
}
