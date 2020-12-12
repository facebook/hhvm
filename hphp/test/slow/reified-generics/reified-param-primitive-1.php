<?hh

class C {}
class D {}

function f<reify T>(<<__Soft>> T $x): <<__Soft>> T { return $x; }
<<__EntryPoint>>
function main_entry(): void {

  set_error_handler(
    (int $errno,
    string $errstr,
    string $errfile,
    int $errline,
    darray $errcontext
    ) ==> {
      echo "ERROR: "; var_dump($errstr);
      return true;
    }
  );

  f<C>(new C());
  f<D>(new C());

  f<shape('a' => C)>(shape('a' => new C()));
  f<shape('a' => D)>(shape('a' => new C()));
}
