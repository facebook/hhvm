<?php
namespace foo\bar\baz {
  require_once ($GLOBALS['HACKLIB_ROOT']);
  function display($x) {
    echo ($x);
  }
}
namespace cdef {
  class C {
    public function foo() {
      return 5;
    }
  }
  class D {
    public $state = "";

    public function __construct($foo, $bar) {
      return $this->state = $foo.$bar;
    }
  }
}
namespace zork {
  \foo\bar\baz\display(\hacklib_id(new \cdef\C())->foo());
  \foo\bar\baz\display(\hacklib_id(new \cdef\D("rin tin ", "tin"))->state);
  \foo\bar\baz\display("\n");
}
