<?hh

function gen1() :AsyncGenerator<mixed,mixed,void>{
    return; // CONST
    yield;
}

function gen2() :AsyncGenerator<mixed,mixed,void>{
    return "str"; // CONST
    yield;
}

function gen3($var) :AsyncGenerator<mixed,mixed,void>{
    return $var; // CV
    yield;
}

function gen4($obj) :AsyncGenerator<mixed,mixed,void>{
    return $obj->prop; // VAR
    yield;
}

function gen5($val) :AsyncGenerator<mixed,mixed,void>{
    return (int) $val; // TMP
    yield;
}


<<__EntryPoint>>
function main_get_return_types() :mixed{
  $gen = gen1();
  $gen->next();
  var_dump($gen->getReturn());

  $gen = gen2();
  $gen->next();
  var_dump($gen->getReturn());

  $gen = gen3(vec[1, 2, 3]);
  $gen->next();
  var_dump($gen->getReturn());

  $o = new stdClass();
  $o->prop = 321;
  $gen = gen4($o);
  $gen->next();
  var_dump($gen->getReturn());

  $gen = gen5("42");
  $gen->next();
  var_dump($gen->getReturn());
}
