<?hh

trait MyTrait {}

function f<reify T>(T $x) :mixed{ echo "done\n"; }
function g(MyTrait $x) :mixed{ echo "done\n"; }

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(
    (int $errno,
    string $errstr,
    string $errfile,
    int $errline,
    darray $errcontext
    ) ==> {
      echo $errstr."\n";
      throw new Exception();
    }
  );
  try { g(1); } catch (Exception $_) {}
  try { f<MyTrait>(1); } catch (Exception $_) {}
}
