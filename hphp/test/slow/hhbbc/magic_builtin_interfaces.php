<?hh

function instance_of_test(varray $x) {
  echo '==================== ', __FUNCTION__,' ====================', "\n";
  var_dump($x is \HH\Traversable);
  var_dump($x is Traversable); // autoimported
  var_dump($x is \HH\Container);
  var_dump($x is Container);   // autoimported
  var_dump($x is XHPChild);
  var_dump($x is Stringish);
  var_dump($x is \HH\KeyedTraversable);
  var_dump($x is KeyedTraversable); // autoimported
  var_dump($x is \HH\KeyedContainer);
  var_dump($x is KeyedContainer);   // autoimported
}

function type_hint_container(\HH\KeyedContainer $x) {
  echo '==================== ', __FUNCTION__,' ====================', "\n";
  var_dump(is_array($x));
}

function type_hint_traversable(\HH\KeyedTraversable $x) {
  echo '==================== ', __FUNCTION__,' ====================', "\n";
  var_dump(is_array($x));
}

function type_hint_stringish(Stringish $x) {
  echo '==================== ', __FUNCTION__,' ====================', "\n";
  var_dump($x is Stringish);
  var_dump($x is XHPChild);
  var_dump(is_string($x));
}

class C {
  public function __toString() {
    return 'C';
  }
}

function main() {
  instance_of_test(varray[1,2,3]);
  type_hint_traversable(varray[1,2,3]);
  type_hint_container(varray[1,2,3]);

  $c = new C();
  type_hint_stringish($c);
}

<<__EntryPoint>>
function main_magic_builtin_interfaces() {
main();
}
