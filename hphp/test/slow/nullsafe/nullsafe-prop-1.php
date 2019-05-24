<?hh

// --------------------------------------------
// 1. Basic nullsafe, base is in a local var
// --------------------------------------------

function basic() {
  echo "= ".__FUNCTION__.":\n\n";
  $x = null;
  var_dump($x?->y);
  echo "\n";
}

// Non-short-circuit.

function sideEffect($fn) {
  echo "sideEffect is called from '$fn'\n";
}

// --------------------------------------------
// 2. Nested with a side effect
// --------------------------------------------

function nested() {
  echo "= ".__FUNCTION__.":\n\n";
  $x = null;
  var_dump($x?->y?->z(sideEffect(__FUNCTION__)));
  echo "\n";
}

// --------------------------------------------
// 3. Not null
// --------------------------------------------

class X {
  public $q = 'q';
  function __construct() {
    $this->y = new Y;
  }
}

class Y {
  function z() {
    return 'z';
  }
}

function notNull() {
  echo "= ".__FUNCTION__.":\n\n";
  $x = new X;
  var_dump($x?->y?->z(sideEffect(__FUNCTION__)));
  var_dump($x->q);
  echo "\n";
}

// --------------------------------------------
// 4. Base is on the stack
// --------------------------------------------

function nonLocalBase() {
  echo "= ".__FUNCTION__.":\n\n";
  var_dump((() ==> null)()?->y); // null
  var_dump((() ==> new X)()?->y?->z(sideEffect(__FUNCTION__))); // 10
  echo "\n";
}

// --------------------------------------------
// 6. XHP attribute
// --------------------------------------------

function xhpAttr() {
  echo "= ".__FUNCTION__.":\n\n";
  $x = null;
  var_dump($x?->:foo);
  echo "\n";
}

// --------------------------------------------
// 7. isset
// --------------------------------------------

function issetProp() {
  echo "= ".__FUNCTION__.":\n\n";
  $x = null;
  var_dump(isset($x?->foo)); // false
  var_dump($x); // null
  echo "\n";
}

// --------------------------------------------
// 8. empty
// --------------------------------------------

function emptyProp() {
  echo "= ".__FUNCTION__.":\n\n";
  $x = null;
  var_dump(!($x?->foo ?? false)); // true
  var_dump($x); // null
  echo "\n";
}


<<__EntryPoint>>
function main_nullsafe_prop_1() {
basic();
nested();
notNull();
nonLocalBase();
xhpAttr();
issetProp();
emptyProp();
}
