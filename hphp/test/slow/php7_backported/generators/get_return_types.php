<?hh

function gen1() {
    return; // CONST
    yield;
}

function gen2() {
    return "str"; // CONST
    yield;
}

function gen3($var) {
    return $var; // CV
    yield;
}

function gen4($obj) {
    return $obj->prop; // VAR
    yield;
}

function gen5($val) {
    return (int) $val; // TMP
    yield;
}


<<__EntryPoint>>
function main_get_return_types() {
  $gen = gen1();
  $gen->next();
  var_dump($gen->getReturn());

  $gen = gen2();
  $gen->next();
  var_dump($gen->getReturn());

  $gen = gen3(varray[1, 2, 3]);
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
