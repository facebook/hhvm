<?hh

class C2 {
  function inner() {
    var_dump(inst_meth($this, __FUNCTION__));
  }
}
