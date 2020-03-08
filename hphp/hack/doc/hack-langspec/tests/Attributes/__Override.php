<?hh // strict

namespace NS_Override;

// --------------------------------------------------------

class Button {
  public function draw(): void { /* ... */ }
  public static function sf(): void { /* ... */ }
}

// If parent class has no function to be overridden (or extends clause omitted), get
// NS_Override\CustomButton::draw() is marked as override; no non-private parent definition 
// found or overridden parent is defined in non-<?hh code 

class CustomButton extends Button {
  <<__Override>>
  public function draw(): void { /* ... */ }
  <<__Override>>
  public static function sf(): void { /* ... */ }
}

// --------------------------------------------------------

trait T1 {
  <<__Override>> // checked on use classes
  public function foo(): void {}
}

class C1 {
//  use T1; // error! foo is not an override
}

class C2 {
  public function foo(): void {}
}

class C3 extends C2 {
  use T1; // OK! C2's implementation is being overridden
}

// --------------------------------------------------------

interface I1 {
  public function f(): void;
}

// Without I1, I2 causes
// NS_Override\I2::f() is marked as override; no non-private parent definition found or overridden
// parent is defined in non-<?hh code

interface I2 extends I1 {
  <<__Override>>
  public function f(): void;
}

class C implements I2 {
  public function f(): void {}
}

// --------------------------------------------------------

//<<__Override, Attr2(3, true)>>	// hmmm! accepted
<<__Override(3), Attr2(3, true)>>	// hmmm! accepted
function f1(): void { echo "Inside " . __FUNCTION__ . "\n"; }

function main(): void {
  echo "\n============== top-level function =====================\n\n";

  f1();
  $rf = new \ReflectionFunction('\NS_Override\f1');
  $attr1 = $rf->getAttribute('__Override');	// hmmm!
  var_dump($attr1);
  $attr2 = $rf->getAttribute('Attr2');
  var_dump($attr2);
}

/* HH_FIXME[1002] call to main in strict*/
main();
