<?hh

class C3 {
  function inner() {
    var_dump(inst_meth($this, __FUNCTION__));
  }
}
