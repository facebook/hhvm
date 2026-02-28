<?hh

<<__Native>>
function get_int(): int;

<<__Native>>
function get_void(): void;

<<__Native>>
function get_mixed(): mixed;

<<__NativeData>>
class Foobar {

  <<__Native>>
  public function getString(): string;

}

<<__NativeData>>
abstract class Baz {

  <<__Native>>
  public function getString(): string;

  abstract public function getNothing(): nothing;

}

<<__NativeData>>
class BingParent {
  <<__Native>>
  public function __construct(): void;
}

<<__NativeData>>
class Bing extends BingParent {
  <<__Native>>
  public function __construct(): void;
}
