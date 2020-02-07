<?hh

final class MyClass {
  public function get_my_class(bool $x): ?MyClass {
    echo "Inside get_my_class\n";
    return $x ? new MyClass() : null;
  }

  public function bar(int $x): void {
    echo "Inside bar\n";
  }
}

function call_func(?(function(int): void) $z): void {
  if ($z !== null) {
    $z(1);
  } else {
    echo "Got null\n";
  }
}

<<__EntryPoint>>
function test(): void {
  $z = new MyClass();

  call_func($z->get_my_class(true)?->bar<>);

  call_func($z->get_my_class(false)?->bar<>);
}
