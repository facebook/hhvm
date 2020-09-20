<?hh

final class Constants {
  public static function gen() {
    yield 'foo';
  }
}

<<__EntryPoint>>
function main() {
  $g = Constants::gen();
  $g->next();
  var_dump($g->current());
}
