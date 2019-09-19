<?hh

trait MyTrait {}

function f<reify T>(T $x) { echo "done\n"; }
function g(MyTrait $x) { echo "done\n"; }

<<__EntryPoint>>
function main() {
  set_error_handler(
    (int $errno,
    string $errstr,
    string $errfile,
    int $errline,
    array $errcontext
    ) ==> {
      echo $errstr."\n";
      throw new Exception();
    }
  );
  try { g(1); } catch (Exception $_) {}
  try { f<MyTrait>(1); } catch (Exception $_) {}
}
