<?hh

class MyClass {
  public function __toDebugDisplay(): string {
    return 'hello';
  }
}

<<__EntryPoint>>
function main() {
  $o = new MyClass();
  debugger_dump($o);
}
