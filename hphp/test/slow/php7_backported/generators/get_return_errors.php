<?hh

function gen1() :AsyncGenerator<mixed,mixed,void>{
    yield 1;
    yield 2;
    return 3;
}

function gen2() :AsyncGenerator<mixed,mixed,void>{
    throw new Exception("gen2() throw");
    yield 1;
    return 2;
}

function gen3() :AsyncGenerator<mixed,mixed,void>{
    yield;
    return 1;
}


<<__EntryPoint>>
function main_get_return_errors() :mixed{
  $gen = gen1();
  $gen->next();
  try {
      // Generator hasn't reached the "return" yet
      $gen->getReturn();
  } catch (Exception $e) {
      echo $e->getMessage(), "\n";
  }

  $gen = gen2();
  try {
      $gen->next();
  } catch (Exception $e) {
      echo $e->getMessage(), "\n";
  }
  try {
      // Generator has been aborted as a result of an exception
      $gen->getReturn();
  } catch (Exception $e) {
      echo $e->getMessage(), "\n";
  }

  $gen = gen3();
  $gen->next();
  try {
      $gen->throw(new Exception("gen3() throw"));
  } catch (Exception $e) {
      echo $e->getMessage(), "\n";
  }
  try {
      // Generator has been aborted as a result of an exception
      // that was injected using throw()
      $gen->getReturn();
  } catch (Exception $e) {
      echo $e->getMessage(), "\n";
  }
}
