<?hh
  function foo($x) {
    return $x + 1;
  }

  function throwSomething() {

    throw new Exception($GLOBALS['_']);
  }

  function printSomething() {

    echo $GLOBALS['_'];
    return "also returned something";
  }
