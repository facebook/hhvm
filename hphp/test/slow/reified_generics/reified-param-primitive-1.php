<?hh

class C {}
class D {}

function f<reify T>(@T $x): @T { return $x; }
<<__EntryPoint>>
function main_entry(): void {

  set_error_handler(
    (int $errno,
    string $errstr,
    string $errfile,
    int $errline,
    array $errcontext
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
