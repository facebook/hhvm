<?hh

namespace {
  var_dump(\HH\meth_caller(A::class, "f"));
  $f = () ==> {
    var_dump(\HH\meth_caller(B::class, "f"));
  };
  $f();

  function afunc() {
    var_dump(\HH\meth_caller(C::class, "f"));
  }
  afunc();

  class Acls {
    function bfunc() {
      var_dump(\HH\meth_caller(D::class, "f"));
    }
  }
  new Acls()->bfunc();
}

namespace Ans {
  var_dump(\HH\meth_caller(A::class, "f"));
  $f = () ==> {
    var_dump(\HH\meth_caller(B::class, "f"));
  };
  $f();

  function afunc() {
    var_dump(\HH\meth_caller(C::class, "f"));
  }
  afunc();

  class Acls {
    function bfunc() {
      var_dump(\HH\meth_caller(D::class, "f"));
    }
  }
  new Acls()->bfunc();
}
