<?php 

interface ITest {
}

interface IFoo extends ITest {
}

class Base {}

class foo extends Base implements ITest {
}

class bar extends foo implements IFoo {
}

<<__EntryPoint>>
function main() {
  var_dump(new foo instanceof Base);
  var_dump(new foo instanceof ITest);
  var_dump(new foo instanceof IFoo);

  var_dump(new bar instanceof Base);
  var_dump(new bar instanceof ITest);
  var_dump(new bar instanceof IFoo);
}
