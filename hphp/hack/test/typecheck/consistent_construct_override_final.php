<?hh

<<__ConsistentConstruct>>
abstract class Foo {
  final protected function __construct() {}

}

final class Bar extends Foo {
  protected function __construct() {
    parent::__construct();
  }
}
