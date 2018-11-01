<?hh //strict

class Example {
  public static function __callStatic(string $name, string $args): string {
    return 'hey';
  }
}
function test(): void {
  $example = new Example();
  Example::__callStatic('three', 'four');
}
