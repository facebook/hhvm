<?hh // should fail

interface I {
  function query();
}

class UserClass {
  function query($arg1) { // Not a built-in function => full check

  }
}

class C extends UserClass implements I {
}


<<__EntryPoint>>
function main_03() {
print("Success\n");
}
