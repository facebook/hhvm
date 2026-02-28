<?hh

final class Constants {
  static public function genA() :AsyncGenerator<mixed,mixed,void>{
    yield 'foo';
  }
  static public function genB($g) :AsyncGenerator<mixed,mixed,void>{
    $g->next();
    yield $g->current();
  }
  static public function genC($g) :AsyncGenerator<mixed,mixed,void>{
    $g->next();
    yield $g->current();
  }
  static public function genD($g) :AsyncGenerator<mixed,mixed,void>{
    $g->next();
    yield $g->current();
  }
  static public function genE($g) :AsyncGenerator<mixed,mixed,void>{
    $g->next();
    yield $g->current();
  }
  static public function genF($g) :AsyncGenerator<mixed,mixed,void>{
    $g->next();
    yield $g->current();
  }
  static public function genG($g) :AsyncGenerator<mixed,mixed,void>{
    $g->next();
    yield $g->current();
  }
  static public function genH($g) :AsyncGenerator<mixed,mixed,void>{
    $g->next();
    yield $g->current();
  }
}

function main() :mixed{
  $g = Constants::genA();
  $g = Constants::genB($g);
  $g = Constants::genC($g);
  $g = Constants::genD($g);
  $g = Constants::genE($g);
  $g = Constants::genF($g);
  $g = Constants::genG($g);
  $g = Constants::genH($g);
  $g->next();
  var_dump($g->current());
}


<<__EntryPoint>>
function main_multi_static() :mixed{
main();
}
