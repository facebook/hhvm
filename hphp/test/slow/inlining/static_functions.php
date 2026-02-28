<?hh

final class Constants {
  public static function gen1() :AsyncGenerator<mixed,mixed,void>{
    yield 'foo';
  }

  public static function gen2() :AsyncGenerator<mixed,mixed,void>{
    yield 'bar';
  }

  public static function gen3($s) :AsyncGenerator<mixed,mixed,void>{
    yield $s;
  }
}

<<__EntryPoint>>
function main() :mixed{
  $g = Constants::gen1();
  $g->next();
  var_dump($g->current());
  $g = Constants::gen2();
  $g->next();
  var_dump($g->current());
  $g = Constants::gen3("baz");
  $g->next();
  var_dump($g->current());
}
