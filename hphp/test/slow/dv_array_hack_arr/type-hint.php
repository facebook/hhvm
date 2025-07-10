<?hh

<<__DynamicallyCallable>> function takes_varray(varray $a) :mixed{}
<<__DynamicallyCallable>> function takes_nullable_varray(?varray $a) :mixed{}
<<__DynamicallyCallable>> function takes_darray(darray $a) :mixed{}
<<__DynamicallyCallable>> function takes_nullable_darray(?darray $a) :mixed{}
<<__DynamicallyCallable>> function takes_varray_or_darray(varray_or_darray $a) :mixed{}
<<__DynamicallyCallable>> function takes_nullable_varray_or_darray(?varray_or_darray $a) :mixed{}
<<__DynamicallyCallable>> function takes_vec_or_dict(vec_or_dict $a) :mixed{}
<<__DynamicallyCallable>> function takes_nullable_vec_or_dict(?vec_or_dict $a) :mixed{}

<<__DynamicallyCallable>> function takes_varray2(varray<int> $a) :mixed{}
<<__DynamicallyCallable>> function takes_darray2(darray<int, string> $a) :mixed{}

<<__DynamicallyCallable>> function returns_varray($a): varray { return $a; }
<<__DynamicallyCallable>> function returns_nullable_varray($a): ?varray { return $a; }
<<__DynamicallyCallable>> function returns_darray($a): darray { return $a; }
<<__DynamicallyCallable>> function returns_nullable_darray($a): ?darray { return $a; }
<<__DynamicallyCallable>> function returns_varray_or_darray($a): varray_or_darray { return $a; }
<<__DynamicallyCallable>> function returns_nullable_varray_or_darray($a): ?varray_or_darray { return $a; }
<<__DynamicallyCallable>> function returns_vec_or_dict($a): vec_or_dict { return $a; }
<<__DynamicallyCallable>> function returns_nullable_vec_or_dict($a): ?vec_or_dict { return $a; }

class Foo {} <<__DynamicallyCallable>> function takes_bool(bool $v) :mixed{}
<<__DynamicallyCallable>> function takes_int(int $v) :mixed{}
<<__DynamicallyCallable>> function takes_str(string $v) :mixed{}
<<__DynamicallyCallable>> function takes_vec(vec $v) :mixed{}
<<__DynamicallyCallable>> function takes_dict(dict $v) :mixed{}
<<__DynamicallyCallable>> function takes_keyset(keyset $v) :mixed{}
<<__DynamicallyCallable>> function takes_resource(resource $v) :mixed{}
<<__DynamicallyCallable>> function takes_obj(object $v) :mixed{}
<<__DynamicallyCallable>> function takes_foo(Foo $v) :mixed{}
<<__DynamicallyCallable>> function takes_vector(Vector $v) :mixed{}
<<__DynamicallyCallable>> function takes_map(Map $v) :mixed{}
<<__DynamicallyCallable>> function takes_mixed(mixed $v) :mixed{}
<<__DynamicallyCallable>> function takes_traversable(Traversable $v) :mixed{}
<<__DynamicallyCallable>> function takes_KeyedContainer(KeyedContainer $v) :mixed{}

<<__DynamicallyCallable>> function returns_bool($v): bool { return $v; }
<<__DynamicallyCallable>> function returns_int($v): int { return $v; }
<<__DynamicallyCallable>> function returns_str($v): string { return $v; }
<<__DynamicallyCallable>> function returns_vec($v): vec { return $v; }
<<__DynamicallyCallable>> function returns_dict($v): dict { return $v; }
<<__DynamicallyCallable>> function returns_keyset($v): keyset { return $v; }
<<__DynamicallyCallable>> function returns_resource($v): resource { return $v; }
<<__DynamicallyCallable>> function returns_obj($v): object { return $v; }
<<__DynamicallyCallable>> function returns_foo($v): Foo { return $v; }
<<__DynamicallyCallable>> function returns_vector($v): Vector { return $v; }
<<__DynamicallyCallable>> function returns_map($v): Map { return $v; }
<<__DynamicallyCallable>> function returns_mixed($v): mixed { return $v; }
<<__DynamicallyCallable>> function returns_traversable($v): Traversable { return $v; }
<<__DynamicallyCallable>> function returns_KeyedContainer($v): KeyedContainer { return $v; }

function test1() :mixed{
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
        new stdClass(),
        vec[],
        vec[1, 2, 3],
        dict[],
        dict['a' => 1, 'b' => 2, 'c' => 3]
      ]
    );

  $funcs = __hhvm_intrinsics\launder_value(
    vec[
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

function test2() :mixed{
  $values = __hhvm_intrinsics\launder_value(
    vec[
      vec[1, 2, 3],
      dict[100 => 200]
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
      'takes_KeyedContainer',

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
      'returns_KeyedContainer'
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
function main_type_hint() :mixed{
set_error_handler(
  (int $errno,
   string $errstr,
   string $errfile,
   int $errline,
   mixed $_errcontext,
  ) ==> {
    throw new Exception($errstr);
  }
);
;

test1();
test2();
}
