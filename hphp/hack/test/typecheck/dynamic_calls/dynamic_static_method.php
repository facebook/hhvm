<?hh

final class WilfredDemo2 {
  public static mixed $f = null;

  public static function accessMethodDynamically(): void {
    WilfredDemo2::$f = () ==> {};

    $f = 'no_such_method';
    WilfredDemo2::$f();
  }
}

<<__EntryPoint>>
function main():void {
  WilfredDemo2::accessMethodDynamically();
}
