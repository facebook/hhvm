<?hh
  function foo($x) {
    return $x + 1;
  }

  function throwSomething() {

    throw new Exception($_ ?? 'I want to see this');
  }

  function printSomething() {

    echo $_ ?? 'I want to see this';
    return "also returned something";
  }
