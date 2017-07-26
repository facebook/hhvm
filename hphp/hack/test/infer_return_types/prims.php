<?hh

  function f () {
    return 1;
    return 1.0;
  }

  function g () {
    return 'str';
    return 1;
    }

  class C  {
    public function meth1 () {
      return 'one';
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
