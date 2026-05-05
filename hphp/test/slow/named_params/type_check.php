<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function try_func($f) :mixed{
  try {
    $f();
  } catch (TypeError $e) {
    printf("Caught %s: %s\n", get_class($e), $e->getMessage());
  }
}

function foo(named int $a, named string $b) {
    var_dump(vec[$a, $b]);
}


function bar(named int $a = 0, named string $b = "") {
    var_dump(vec[$a, $b]);
}

class C {
  public function foo(named int $a, named string $b) {
    var_dump(vec[$a, $b]);
  }

  public function bar(named int $a = 0, named string $b = "") {
    var_dump(vec[$a, $b]);
  }
}

<<__EntryPoint>>
function main() {
  set_error_handler(
    function(int $errno, string $errmsg, mixed ...$args) {
      throw new TypeError($errmsg);
    },
    E_RECOVERABLE_ERROR,
  );
  try_func(() ==> foo(a=1, b=""));
  try_func(() ==> foo(a=1, b=2));
  try_func(() ==> foo(a="", b=""));
  try_func(() ==> foo(a="", b=1));

  try_func(() ==> bar(a=""));
  try_func(() ==> bar(b=1));
  try_func(() ==> bar(b=""));
  try_func(() ==> bar(a=1, b=2));
  try_func(() ==> bar(a="", b=2));
  try_func(() ==> bar(a=1, b=2));

  try_func(() ==> (new C())->foo(a=1, b=""));
  try_func(() ==> (new C())->foo(a=1, b=2));
  try_func(() ==> (new C())->foo(a="", b=""));
  try_func(() ==> (new C())->foo(a="", b=1));

  try_func(() ==> (new C())->bar(a=""));
  try_func(() ==> (new C())->bar(b=1));
  try_func(() ==> (new C())->bar(b=""));
  try_func(() ==> (new C())->bar(a=1, b=2));
  try_func(() ==> (new C())->bar(a="", b=2));
  try_func(() ==> (new C())->bar(a=1, b=2));
}
