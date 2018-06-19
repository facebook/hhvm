<?hh

  function f () {
    if (true) {
      return 1;
    }
    return 1.0;
  }

  function g () {
    if (true) {
      return 'str';
    }
    return 1;
    }

  class C  {
    public function meth1 () {
      if (true) {
        return 'one';
      }
      return 'two';
    }

    public function meth2 () : int {
      return 1;
    }
  }

  function h (int $x) {
    return $x+2;
  }

  function k (int $x) {
    $x+2;
  }
