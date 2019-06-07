<?hh

class mystring {
}

class myint {
}

class myfloat {
}

class mybool {
}

class myresource {
}

class myarraykey {
}

class mynum {
}

function test_hh_string(@HH\string $foo) { var_dump($foo); }
function test_hh_int(@HH\int $foo) { var_dump($foo); }
function test_hh_float(@HH\float $foo) { var_dump($foo); }
function test_hh_bool(@HH\bool $foo) { var_dump($foo); }
function test_hh_resource(@HH\resource $foo) { var_dump($foo); }
function test_hh_arraykey(@HH\arraykey $foo) { var_dump($foo); }
function test_hh_num(@HH\num $foo) { var_dump($foo); }

function test_string(@string $foo) { var_dump($foo); }
function test_int(@int $foo) { var_dump($foo); }
function test_float(@float $foo) { var_dump($foo); }
function test_bool(@bool $foo) { var_dump($foo); }
function test_resource(@resource $foo) { var_dump($foo); }
function test_arraykey(@arraykey $foo) { var_dump($foo); }
function test_num(@num $foo) { var_dump($foo); }


// It's important that we're in a "<?hh" file and that HH syntax
// is not enabled, otherwise this test would fatal at parse time

<<__EntryPoint>>
function main_scalar_names() {
set_error_handler(
  function(
    HH\int $errno,
    HH\string $errstr
  ) {
    printf("ERROR (%d): %s\n ==> ", $errno, $errstr);
    // ON ERROR RESUME NEXT;
    return true;
  }
);

print("--- CORRECT USAGE ---\n");

test_hh_string('123');
test_hh_int(123);
test_hh_float(1.23);
test_hh_bool(true);
test_hh_resource(STDIN);
test_hh_arraykey('herp');
test_hh_arraykey(123);
test_hh_num(123);
test_hh_num(1.23);

test_string('123');
test_int(123);
test_float(1.23);
test_bool(true);
test_resource(STDIN);
test_arraykey('herp');
test_arraykey(123);
test_num(1.23);

print("--- INCORRECT USAGE ---\n");

test_hh_string(new mystring());
test_hh_int(new myint());
test_hh_float(new myfloat());
test_hh_bool(new mybool());
test_hh_resource(new myresource());
test_hh_arraykey(new myarraykey());
test_hh_arraykey(1.23);
test_hh_num(new mynum());
test_hh_num('123');
test_string(new mystring());
test_int(new myint());
test_float(new myfloat());
test_bool(new mybool());
test_resource(new myresource());
test_arraykey(new myarraykey());
test_num(new mynum());
}
