<?hh // should fail

interface I {
  function query(); // HipHop syntax enabled => full check
}

class C extends PDO implements I {
}


<<__EntryPoint>>
function main_02() {
print(print_r(ini_get_all()));

print("Success\n");
}
