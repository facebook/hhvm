<?hh

function gen1() :AsyncGenerator<mixed,mixed,void>{
    return 42;
    yield 24;
}

function gen2() :AsyncGenerator<mixed,mixed,void>{
    yield 24;
    return 42;
}

// ============================================================================
// `gen3` was testing by-reference yields, which PHP 5/7 support but which
// HHVM does not. That test should be added back in when HHVM adds support for
// this feature.
// ============================================================================

// Return types for generators specify the return of the function,
// not of the generator return value, so this code is okay
function gen4() :AsyncGenerator<mixed,mixed,void>{
    yield 24;
    return 42;
}

// Has no explicit return, but implicitly return NULL at the end
function gen5() :AsyncGenerator<mixed,mixed,void>{
    yield 24;
}

// Explicit value-less return also results in a NULL generator
// return value and there is no interference with type hints
function gen6() :AsyncGenerator<mixed,mixed,void>{
    return;
    yield 24;
}


<<__EntryPoint>>
function main_get_return() :mixed{
  $gen = gen1();
  $gen->next();
  var_dump($gen->getReturn());

  $gen = gen2();
  $gen->next();
  var_dump($gen->current());
  $gen->next();
  var_dump($gen->getReturn());

  $gen = gen4();
  $gen->next();
  var_dump($gen->current());
  $gen->next();
  var_dump($gen->getReturn());

  $gen = gen5();
  $gen->next();
  var_dump($gen->current());
  $gen->next();
  var_dump($gen->getReturn());

  $gen = gen6();
  $gen->next();
  var_dump($gen->getReturn());
}
