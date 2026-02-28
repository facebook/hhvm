<?hh

// FILE: cls.php

class Foo<T> {
  function __construct(public $str) {}
}

// FILE: cls.php VERSION: 1

class Foo<reify T> {
  function __construct(public $str) {}
}

// FILE: cls.php VERSION: 3

class Foo<T> {
  function __construct(public $str) {}
}

// FILE: alias.php

type TAlias = int;

// FILE: alias.php VERSION: 2

type TAlias = bool;

// FILE: test.php

function p($s) :mixed{
  $bcs = vec[];
  foreach (explode(' ', $s) as $word) {
    if (strpos($word, 'VerifyParamType') === 0) $bcs[] = $word;
  }
  echo "Verify BCs: ".implode(', ', $bcs)."\n";
}

function infoi(Foo<int> $foo)    :mixed{ p(__hhvm_intrinsics\debug_get_bytecode()); }
function infob(Foo<bool> $foo)   :mixed{ p(__hhvm_intrinsics\debug_get_bytecode()); }
function infoa(Foo<TAlias> $foo) :mixed{ p(__hhvm_intrinsics\debug_get_bytecode()); }

function testi(Foo<int> $foo)    :mixed{ echo "testi(".$foo->str.") = ok\n"; }
function testb(Foo<bool> $foo)   :mixed{ echo "testb(".$foo->str.") = ok\n"; }
function testa(Foo<TAlias> $foo) :mixed{ echo "testa(".$foo->str.") = ok\n"; }

function deps() :mixed{
  echo "Deps: ".implode(', ', __hhvm_intrinsics\debug_file_deps())."\n";
}

// FILE: main.php

function go($foo) :mixed{
  $str = $foo->str;
  try { testi($foo); } catch (Exception $e) { echo "testi($str) = ex\n"; }
  try { testb($foo); } catch (Exception $e) { echo "testb($str) = ex\n"; }
  try { testa($foo); } catch (Exception $e) { echo "testa($str) = ex\n"; }
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(() ==> { throw new Exception; }, E_RECOVERABLE_ERROR);

  $r = new ReflectionClass('Foo')->getReifiedTypeParamInfo();
  $r = count($r) > 0 ? $r[0]['is_reified'] : 0;
  $k = type_structure('TAlias')['kind'];
  echo "Foo<T> is ".($r ? "" : "not ")."reified\n";
  echo "TAlias kind is $k\n";

  $fooi = new Foo<int>('Foo<int>');
  $foob = new Foo<bool>('Foo<Bool>');
  $fooa = new Foo<TAlias>('Foo<TAlias>');

  go($fooi); go($foob); go($fooa);
  infoi($fooi); infob($foob); infoa($fooa);
  deps();
}
