<?hh 

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
function main() :mixed{
  var_dump(new foo is Base);
  var_dump(new foo is ITest);
  var_dump(new foo is IFoo);

  var_dump(new bar is Base);
  var_dump(new bar is ITest);
  var_dump(new bar is IFoo);
}
