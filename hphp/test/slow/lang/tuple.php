<?hh

class A {
  function c() :mixed{
    return tuple(1,2);
  }
}

<<__EntryPoint>>
function main_tuple() :mixed{
var_dump((new A)->c());
}
