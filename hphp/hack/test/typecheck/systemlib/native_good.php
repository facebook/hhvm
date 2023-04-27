<?hh

<<__Native>>
function get_int(): int;

<<__Native>>
function get_void(): void;

<<__Native>>
function get_mixed(): mixed;

<<__NativeData('Foobar')>>
class Foobar {

  <<__Native>>
  public function getString(): string;

}

<<__NativeData('Baz')>>
abstract class Baz {

  <<__Native>>
  public function getString(): string;

  abstract public function getNothing(): nothing;

}

<<__NativeData('BingParent')>>
class BingParent {
  <<__Native>>
  public function __construct(): void;
}

<<__NativeData('Bing')>>
class Bing extends BingParent {
  <<__Native>>
  public function __construct(): void;
}
