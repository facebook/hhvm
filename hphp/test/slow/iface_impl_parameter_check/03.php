<?hh // should fail

interface I {
  function query():mixed;
}

class UserClass {
  function query($arg1) :mixed{ // Not a built-in function => full check

  }
}

class C extends UserClass implements I {
}


<<__EntryPoint>>
function main_03() :mixed{
print("Success\n");
}
