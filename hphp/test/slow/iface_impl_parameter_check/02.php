<?hh // should fail

interface I {
  function query():mixed; // HipHop syntax enabled => full check
}

class C extends PDO implements I {
}


<<__EntryPoint>>
function main_02() :mixed{
print(print_r(ini_get_all()));

print("Success\n");
}
