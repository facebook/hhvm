<?hh

class C {
  function inner() {
    var_dump(inst_meth($this, __FUNCTION__));
  }
}
