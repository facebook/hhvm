<?hh

final class Constants {
  public static function gen() :AsyncGenerator<mixed,mixed,void>{
    yield 'foo';
  }
}

<<__EntryPoint>>
function main() :mixed{
  $g = Constants::gen();
  $g->next();
  var_dump($g->current());
}
