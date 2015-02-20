<?php

// It's important that we're in a "<?php" file and that HH syntax
// is not enabled, otherwise this test would fatal at parse time

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

class string {
}

class int {
}

class float {
}

class bool {
}

class resource {
}

class arraykey {
}

class num {
}

function test_hh_string(HH\string $foo) { var_dump($foo); }
function test_hh_int(HH\int $foo) { var_dump($foo); }
function test_hh_float(HH\float $foo) { var_dump($foo); }
function test_hh_bool(HH\bool $foo) { var_dump($foo); }
function test_hh_resource(HH\resource $foo) { var_dump($foo); }
function test_hh_arraykey(HH\arraykey $foo) { var_dump($foo); }
function test_hh_num(HH\num $foo) { var_dump($foo); }

function test_string(string $foo) { var_dump($foo); }
function test_int(int $foo) { var_dump($foo); }
function test_float(float $foo) { var_dump($foo); }
function test_bool(bool $foo) { var_dump($foo); }
function test_resource(resource $foo) { var_dump($foo); }
function test_arraykey(arraykey $foo) { var_dump($foo); }
function test_num(num $foo) { var_dump($foo); }

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

test_string(new string());
test_int(new int());
test_float(new float());
test_bool(new bool());
test_resource(new resource());
test_arraykey(new arraykey());
test_num(new num());

print("--- INCORRECT USAGE ---\n");

test_hh_string(new string());
test_hh_int(new int());
test_hh_float(new float());
test_hh_bool(new bool());
test_hh_resource(new resource());
test_hh_arraykey(new arraykey());
test_hh_arraykey(1.23);
test_hh_num(new num());
test_hh_num('123');

test_string('123');
test_int(123);
test_float(1.23);
test_bool(true);
test_resource(STDIN);
test_arraykey('herp');
test_arraykey(123);
test_num(1.23);
