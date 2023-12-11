<?hh

function instance_of_test(varray $x) :mixed{
  echo '==================== ', __FUNCTION__,' ====================', "\n";
  var_dump($x is \HH\Traversable);
  var_dump($x is Traversable); // autoimported
  var_dump($x is \HH\Container);
  var_dump($x is Container);   // autoimported
  var_dump($x is XHPChild);
  var_dump($x is Stringish);
  var_dump($x is StringishObject);
  var_dump($x is \HH\KeyedTraversable);
  var_dump($x is KeyedTraversable); // autoimported
  var_dump($x is \HH\KeyedContainer);
  var_dump($x is KeyedContainer);   // autoimported
}

function type_hint_container(\HH\KeyedContainer $x) :mixed{
  echo '==================== ', __FUNCTION__,' ====================', "\n";
  var_dump(is_array($x));
}

function type_hint_traversable(\HH\KeyedTraversable $x) :mixed{
  echo '==================== ', __FUNCTION__,' ====================', "\n";
  var_dump(is_array($x));
}

function type_hint_stringish(Stringish $x) :mixed{
  echo '==================== ', __FUNCTION__,' ====================', "\n";
  var_dump($x is Stringish);
  var_dump($x is XHPChild);
  var_dump(is_string($x));
}

function type_hint_stringishObject(StringishObject $x) :mixed{
  echo '==================== ', __FUNCTION__,' ====================', "\n";
  var_dump($x is StringishObject);
  var_dump($x is XHPChild);
  var_dump(is_string($x));
}

class C {
  public function __toString()[] :mixed{
    return 'C';
  }
}

function main() :mixed{
  instance_of_test(vec[1,2,3]);
  type_hint_traversable(vec[1,2,3]);
  type_hint_container(vec[1,2,3]);

  $c = new C();
  type_hint_stringish($c);
  type_hint_stringishObject($c);
}

<<__EntryPoint>>
function main_magic_builtin_interfaces() :mixed{
main();
}
