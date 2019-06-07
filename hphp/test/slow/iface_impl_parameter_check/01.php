<?hh // should fail

interface I1 {
  // Has less args then the implementing function
  function query(); // Built-in function, relaxed check
}

class C1 extends PDO implements I1 {
}

interface I2 {
  // Unlike the implementing function, doesn't have an arg type
  function query($arg); // Built-in function, relaxed check
}

class C2 extends PDO implements I2 {
}


<<__EntryPoint>>
function main_01() {
print("Success\n");
}
