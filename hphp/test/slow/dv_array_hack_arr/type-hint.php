<?hh

function takes_array(array $a) {}
function takes_nullable_array(?array $a) {}
function takes_varray(varray $a) {}
function takes_nullable_varray(?varray $a) {}
function takes_darray(darray $a) {}
function takes_nullable_darray(?darray $a) {}
function takes_varray_or_darray(varray_or_darray $a) {}
function takes_nullable_varray_or_darray(?varray_or_darray $a) {}
function takes_vec_or_dict(vec_or_dict $a) {}
function takes_nullable_vec_or_dict(?vec_or_dict $a) {}

function takes_varray2(varray<int> $a) {}
function takes_darray2(darray<int, string> $a) {}

function returns_array($a): array { return $a; }
function returns_nullable_array($a): ?array { return $a; }
function returns_varray($a): varray { return $a; }
function returns_nullable_varray($a): ?varray { return $a; }
function returns_darray($a): darray { return $a; }
function returns_nullable_darray($a): ?darray { return $a; }
function returns_varray_or_darray($a): varray_or_darray { return $a; }
function returns_nullable_varray_or_darray($a): ?varray_or_darray { return $a; }
function returns_vec_or_dict($a): vec_or_dict { return $a; }
function returns_nullable_vec_or_dict($a): ?vec_or_dict { return $a; }

class Foo {}function takes_bool(bool $v) {}
function takes_int(int $v) {}
function takes_str(string $v) {}
function takes_vec(vec $v) {}
function takes_dict(dict $v) {}
function takes_keyset(keyset $v) {}
function takes_resource(resource $v) {}
function takes_obj(object $v) {}
function takes_foo(Foo $v) {}
function takes_vector(Vector $v) {}
function takes_map(Map $v) {}
function takes_mixed(mixed $v) {}
function takes_traversable(Traversable $v) {}
function takes_indexish(Indexish $v) {}

function returns_bool($v): bool { return $v; }
function returns_int($v): int { return $v; }
function returns_str($v): string { return $v; }
function returns_vec($v): vec { return $v; }
function returns_dict($v): dict { return $v; }
function returns_keyset($v): keyset { return $v; }
function returns_resource($v): resource { return $v; }
function returns_obj($v): object { return $v; }
function returns_foo($v): Foo { return $v; }
function returns_vector($v): Vector { return $v; }
function returns_map($v): Map { return $v; }
function returns_mixed($v): mixed { return $v; }
function returns_traversable($v): Traversable { return $v; }
function returns_indexish($v): Indexish { return $v; }

function test1() {
  $values =
    __hhvm_intrinsics\launder_value(
      vec[
        null,
        false,
        true,
        'abc',
        123,
        1.234,
        vec[],
        vec[1, 2, 3, 4],
        dict[],
        dict[1 => 'a', 2 => 'b'],
        keyset[],
        keyset[100, 'abc', 200],
        xml_parser_create(),
        new stdclass(),
        [],
        [1, 2, 3, 4],
        varray[],
        varray[1, 2, 3],
        darray[],
        darray['a' => 1, 'b' => 2, 'c' => 3]
      ]
    );

  $funcs = __hhvm_intrinsics\launder_value(
    vec[
      'takes_array',
      'takes_nullable_array',
      'takes_varray',
      'takes_nullable_varray',
      'takes_darray',
      'takes_nullable_darray',
      'takes_varray_or_darray',
      'takes_nullable_varray_or_darray',
      'takes_vec_or_dict',
      'takes_nullable_vec_or_dict',

      'takes_varray2',
      'takes_darray2',

      'returns_array',
      'returns_nullable_array',
      'returns_varray',
      'returns_nullable_varray',
      'returns_darray',
      'returns_nullable_darray',
      'returns_varray_or_darray',
      'returns_nullable_varray_or_darray',
      'returns_vec_or_dict',
      'returns_nullable_vec_or_dict'
    ]
  );

  foreach ($values as $v) {
    foreach ($funcs as $f) {
      try {
        echo "================ $f ============================\n";
        $f($v);
        echo "PASSES\n";
      } catch (Exception $e) {
        var_dump($e->getMessage());
      }
    }
  }
}

function test2() {
  $values = __hhvm_intrinsics\launder_value(
    vec[
      varray[1, 2, 3],
      darray[100 => 200]
    ]
  );

  $funcs = __hhvm_intrinsics\launder_value(
    vec[
      'takes_bool',
      'takes_int',
      'takes_str',
      'takes_vec',
      'takes_dict',
      'takes_keyset',
      'takes_resource',
      'takes_obj',
      'takes_foo',
      'takes_vector',
      'takes_map',
      'takes_mixed',
      'takes_traversable',
      'takes_indexish',

      'returns_bool',
      'returns_int',
      'returns_str',
      'returns_vec',
      'returns_dict',
      'returns_keyset',
      'returns_resource',
      'returns_obj',
      'returns_foo',
      'returns_vector',
      'returns_map',
      'returns_mixed',
      'returns_traversable',
      'returns_indexish'
    ]
  );

  foreach ($values as $v) {
    foreach ($funcs as $f) {
      try {
        echo "================ $f ============================\n";
        $f($v);
        echo "PASSES\n";
      } catch (Exception $e) {
        var_dump($e->getMessage());
      }
    }
  }
}

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_type_hint() {
set_error_handler(
  (int $errno,
   string $errstr,
   string $errfile,
   int $errline,
   array $errcontext
  ) ==> {
    throw new Exception($errstr);
  }
);
;

test1();
test2();
}
