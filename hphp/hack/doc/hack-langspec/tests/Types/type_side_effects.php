<?hh // strict

namespace NS_type_side_effects;

class Cx {
  private ?int $prop1 = 8;		// although starts out holding an int

  public function m(): void {
//    $x = $this->prop1 << 2;		// disallowed; type of $this->prop1 is not int    

    if (is_int($this->prop1)) {		// type side effect occurs; $this->prop1 has type int
      $x = $this->prop1 << 2;		// allowed; type is int 
//      $this->n(); 

// As n could change the contents of $prop1 back to null, checker reports
// "All the local information about the member prop1 has been invalidated during this call.
// $y is no longer guaranteed [to have the same type as before the call]"

      $x = $this->prop1 << 2;		// disallowed; type of $this->prop1 might not be int 
    }
  }

  public function n(): void {
    var_dump($this->prop1);    
    $this->prop1 = null;  	// make it hold an int  
    var_dump($this->prop1);    
  }
}

function F_bool(bool $p1): void {}
function F_n_bool(?bool $p1): void {
//  F_bool($p1);		// type not yet determined

  if (is_bool($p1)) {	// type side effect occurs; $p1 has type bool
    F_bool($p1);	// allowed; type is bool
  } else {		// type side effect occurs; $p1 has type null
//    F_bool($p1);	// disallowed; type is not bool
  }

  if (is_null($p1)) {	// type side effect occurs; $p1 has type null
//  F_bool($p1);	// disallowed; type is not bool
  } else {		// type side effect occurs; $p1 has type bool
    F_bool($p1);	// allowed; type is bool
  }

  if (!is_null($p1)) {	// type side effect occurs; $p1 has type bool
    F_bool($p1);	// allowed; type is bool
  }

  if ($p1 !== null) {	// type side effect occurs; $p1 has type bool
    F_bool($p1);	// allowed; type is bool
  }

  if ($p1) {		// type side effect occurs; $p1 has type bool
    F_bool($p1);	// allowed; type is bool
  } else {
//    F_bool($p1);	// disallowed; type is not bool
  }
}

function F_n_int(?int $p1): void {
//  $x = $p1 % 3;	// disallowed; % not defined for ?int
  if (is_int($p1)) {	// type side effect occurs; $p1 has type int
    $x = $p1 % 3;	// allowed; type is int
  }

  if (is_null($p1)) {	// type side effect occurs; $p1 has type null
//  $x = $p1 % 3;	// disallowed; type is not int
  } else {		// type side effect occurs; $p1 has type int
    $x = $p1 % 3;	// allowed; type is int
  }

  if (!is_null($p1)) {	// type side effect occurs; $p1 has type int
    $x = $p1 % 3;	// allowed; type is int
  }

  if ($p1 !== null) {	// type side effect occurs; $p1 has type int
    $x = $p1 % 3;	// allowed; type is int
  }

  if ($p1) {		// type side effect occurs; $p1 has type int
    $x = $p1 % 3;	// allowed; type is int
  } else {
//    $x = $p1 % 3;	// disallowed; type is null


  if (is_int($p1) && $p1 > 10) {	// type side effect occurs; $p1 has type int
    $x = $p1 % 3;	// accepted; % defined for int
  }

/*
  if (is_int($p1) || $p1 > 10) {	// type side effect not guaranteed to occur
    $x = $p1 % 3;	// rejected
  }
*/

  is_int($p1) ? $x = $p1 % 3 : $x = -1;		// accepted
//  !is_int($p1) ? $x = $p1 % 3 : $x = -1;	// rejected
  }
}

function F_n_float(?float $p1): void {
//  $x = $p1**2;	// disallowed; ** not defined for ?float
  if (is_float($p1)) {	// type side effect occurs; $p1 has type float
    $x = $p1**2;	// allowed; type is float
  }

  if (is_null($p1)) {	// type side effect occurs; $p1 has type null
//  $x = $p1**2;	// disallowed; ** not defined for null
  } else {		// type side effect occurs; $p1 has type float
    $x = $p1**2;	// allowed; type is float
  }

  if (!is_null($p1)) {	// type side effect occurs; $p1 has type float
    $x = $p1**2;	// allowed; type is float
  }

  if ($p1 !== null) {	// type side effect occurs; $p1 has type float
    $x = $p1**2;	// allowed; type is float
  }

  if ($p1) {		// type side effect occurs; $p1 has type int
    $x = $p1**2;	// allowed; type is float
  } else {
//    $x = $p1**2;	// disallowed; type is null
  }
}

function F_n_num(?num $p1): void {
//  $x = $p1**2;	// disallowed; ** not defined for ?num
  if (is_int($p1)) {	// type side effect occurs; $p1 has type num
    $x = $p1**2;	// allowed; type is num
  }

  if (is_float($p1)) {	// type side effect occurs; $p1 has type float
    $x = $p1**2;	// allowed; type is float
  }
	
  if (is_int($p1) || is_float($p1)) {
// ***  $x = $p1**2;	// *** one might think this is okay, but in fact is not
  }

  if (is_null($p1)) {	// type side effect occurs; $p1 has type null
//  $x = $p1**2;	// disallowed; ** not defined for null
  } else {		// type side effect occurs; $p1 has type num
    $x = $p1**2;	// allowed; type is num
  }

  if (!is_null($p1)) {	// type side effect occurs; $p1 has type num
    $x = $p1**2;	// allowed; type is num
  }

  if ($p1 !== null) {	// type side effect occurs; $p1 has type num
    $x = $p1**2;	// allowed; type is num
  }
}

function F_num(num $p1): void {
//  $x = $p1 % 3;	// disallowed; % not defined for num
  if (is_int($p1)) {	// type side effect occurs; $p1 has type int
    $x = $p1 % 3;	// allowed; type is int
  }
}

function F_string(string $p1): void {}
function F_n_string(?string $p1): void {
//  F_string($p1);	// disallowed; function requires a string

  if (is_string($p1)) {	// type side effect occurs; $p1 has type string
    F_string($p1);	// allowed; type is string
  } else {		// type side effect occurs; $p1 has type null
//  F_string($p1);	// disallowed; function requires a string
  }

  if (is_null($p1)) {	// type side effect occurs; $p1 has type null
//  F_string($p1);	// disallowed; function requires a string
  } else {		// type side effect occurs; $p1 has type string
    F_string($p1);	// allowed; type is string
  }

  if (!is_null($p1)) {	// type side effect occurs; $p1 has type string
    F_string($p1);	// allowed; type is string
  }

  if ($p1 !== null) {	// type side effect occurs; $p1 has type string
    F_string($p1);	// allowed; type is string
  }
}

function F_n_arraykey(?arraykey $p1): void {
//  $x = $p1 % 3;	// disallowed; % not defined for ?arraykey
//  F_string($p1);	// disallowed; function requires a string
//  F_arraykey($p1);	// disallowed; function requires an arraykey

  if (is_int($p1)) {	// type side effect occurs; $p1 has type int
    $x = $p1 % 3;	// allowed; type is int
  }

  if (is_string($p1)) {	// type side effect occurs; $p1 has type string
    F_string($p1);	// allowed; type is string
  }

  if ($p1 !== null) {	// type side effect occurs; $p1 has type arraykey
    F_arraykey($p1);	// allowed; type is arraykey
  }
}

function F_arraykey(arraykey $p1): void {
//  $x = $p1 % 3;	// disallowed; % not defined for arraykey
//  F_string($p1);	// disallowed; function requires a string

  if (is_int($p1)) {	// type side effect occurs; $p1 has type int
    $x = $p1 % 3;	// allowed; type is int
  }

  if (is_string($p1)) {	// type side effect occurs; $p1 has type string
    F_string($p1);	// allowed; type is string
  }
}

function F_array(array<int> $p1): void {}
function F_n_array(?array<int> $p1): void {
//  F_array($p1);	// disallowed; function requires an array<int>

  if (is_array($p1)) {	// type side effect occurs; $p1 has type array<int>
    F_array($p1);	// allowed; type is array<int>
  } else {		// type side effect occurs; $p1 has type null
//  F_array($p1);	// disallowed; function requires an array<int>
  }

  if (is_null($p1)) {	// type side effect occurs; $p1 has type null
//  F_array($p1);	// disallowed; function requires an array<int>
  } else {		// type side effect occurs; $p1 has type array<int>
    F_array($p1);	// allowed; type is array<int>
  }

  if (!is_null($p1)) {	// type side effect occurs; $p1 has type array<int>
    F_array($p1);	// allowed; type is array<int>
  }

  if ($p1 !== null) {	// type side effect occurs; $p1 has type array<int>
    F_array($p1);	// allowed; type is array<int>
  }
}

function F_resource(resource $p1): void {}
function F_n_resource(?resource $p1): void {
//  F_resource($p1);	// disallowed; function requires a resource

  if (is_resource($p1)) {// type side effect occurs; $p1 has type resource
    F_resource($p1);	// allowed; type is resource
  } else {		// type side effect occurs; $p1 has type null
//  F_resource($p1);	// disallowed, type is not resource
  }

  if (is_null($p1)) {	// type side effect occurs; $p1 has type null
//  F_resource($p1);	// type is not resource
  } else {		// type side effect occurs; $p1 has type resource
    F_resource($p1);	// allowed; type is resource
  }

  if (!is_null($p1)) {	// type side effect occurs; $p1 has type resource
    F_resource($p1);	// allowed; type is resource
  }

  if ($p1 !== null) {	// type side effect occurs; $p1 has type resource
    F_resource($p1);	// allowed; type is resource
  }
}

class C {
  public function f(): void {}
}

function F_n_classC(?C $p1): void {
//  $p1->f();		// disallowed; function requires a C


// *** is_object == true does NOT determine the type as being usable as an object

  if (is_object($p1)) {
//  $p1->f();		// disallowed
  } else {
//  $p1->f();		// disallowed
  }

  if (is_null($p1)) {	// type side effect occurs; $p1 has type null
//  	$p1->f();	// disallowed; type is not C
  } else {
    $p1->f();		// allowed; type is C
  }

  if (!is_null($p1)) {	// type side effect occurs; $p1 has type C
    $p1->f();		// allowed; type is C
  }

  if ($p1 !== null) {	// type side effect occurs; $p1 has type C
    $p1->f();		// allowed; type is C
  }
}

function F_tuple((int, int) $p1): void {}
function F_n_tuple(?(int, int) $p1): void {
//  F_tuple($p1);	// disallowed; function requires a tuple

  if (is_null($p1)) {	// type side effect occurs; $p1 has type null
//  F_tuple($p1);	// disallowed; type is not tuple
  } else {		// type side effect occurs; $p1 has type tuple
    F_tuple($p1);	// allowed; type is tuple
  }

  if (!is_null($p1)) {	// type side effect occurs; $p1 has type null
    F_tuple($p1);	// allowed; type is tuple
  }

  if ($p1 !== null) {	// type side effect occurs; $p1 has type null
    F_tuple($p1);	// allowed; type is tuple
  }
}

type MyPoint = shape('x' => int, 'y' => int);
function F_shape(MyPoint $p1): void {}
function F_n_shape(?MyPoint $p1): void {
//  F_shape($p1);	// disallowed; function requires a shape

  if (is_null($p1)) {	// type side effect occurs; $p1 has type null
//  F_shape($p1);	// disallowed; type is not shape
  } else {		// type side effect occurs; $p1 has type MyPoint
    F_shape($p1);	// allowed; type is shape
  }

  if (!is_null($p1)) {	// type side effect occurs; $p1 has type MyPoint
    F_shape($p1);	// allowed; type is shape
  }

  if ($p1 !== null) {	// type side effect occurs; $p1 has type MyPoint
    F_shape($p1);	// allowed; type is shape
  }
}

function F_closure((function (int): int) $p1): void {}
function F_n_closure(?(function (int): int) $p1): void {
//  F_closure($p1);	// disallowed; function requires a closure

  if (is_callable($p1)) {

// *** is_closure == true does NOT determine the type as being usable as a closure

//  F_closure($p1);	// disallowed
  } else {
//  F_closure($p1);	// disallowed
  }

  if (is_null($p1)) {	// type side effect occurs; $p1 has type null
//  F_closure($p1);	// type is not closure
  } else {		// type side effect occurs; $p1 has type closure
    F_closure($p1);	// allowed; type is closure
  }

  if (!is_null($p1)) {	// type side effect occurs; $p1 has type closure
    F_closure($p1);	// allowed; type is closure
  }

  if ($p1 !== null) {	// type side effect occurs; $p1 has type closure
    F_closure($p1);	// allowed; type is closure
  }
}

class Button {
  public function f(): void {}
}

class CustomButton extends Button {
  public function g(): void {}
}

function F_Button(Button $p1): void {}
function F_CustomButton(CustomButton $p1): void {}
function F_n_class_hier(?Button $p1): void {
//  F_Button($p1);		// disallowed; function requires a Button
//  F_CustomButton($p1);	// disallowed; function requires a CustomButton

  if (is_null($p1)) {		// type side effect occurs; $p1 has type null
//  F_Button($p1);		// type is not Button
  } else {			// type side effect occurs; $p1 has type Button
    F_Button($p1);		// allowed; type is some kind of Button
//  F_CustomButton($p1);	// but not necesarily a CustomButton
    if ($p1 instanceof CustomButton) { // type side effect occurs; $p1 has type CustomButton
      F_CustomButton($p1);	// allowed; type is CustomButton
    }
  }

  if (!is_null($p1)) {		// type side effect occurs; $p1 has type Button
    F_Button($p1);		// allowed; type is some kind of Button
//  F_CustomButton($p1);	// but not necesarily a CustomButton
    if ($p1 instanceof CustomButton) { // type side effect occurs; $p1 has type CustomButton
      F_CustomButton($p1);	// allowed; type is CustomButton
    }
  }

  if ($p1 !== null) {		// type side effect occurs; $p1 has type Button
    F_Button($p1);		// allowed; type is some kind of Button
//  F_CustomButton($p1);	// but not necesarily a CustomButton
    if ($p1 instanceof CustomButton) { // type side effect occurs; $p1 has type CustomButton
      F_CustomButton($p1);	// allowed; type is CustomButton
    }
  }
}

function getValue(): ?int {
  return null;
//  return 10; 
}

function main(): void {
  $c = new Cx();
  $c->m();

  $v = getValue();
  if ($v !== null) {
    $x = $v % 3;	// accepted; % defined for int
    echo "\$x = $x\n";
  }
//  $x = $v % 3;	// rejected

  $v = getValue();
  while ($v !== null) {
    $x = $v % 3;	// accepted; % defined for int
    echo "\$x = $x\n";
    $v = null;
  }
//  $x = $v % 3;	// rejected

  $v = getValue();
  for (;$v !== null;) {
    $x = $v % 3;	// accepted; % defined for int
    echo "\$x = $x\n";
    $v = null;
  }
//  $x = $v % 3;	// rejected

/*
  $v = getValue();
  do {
    $x = $v % 3;	// rejected; 1st time through could be null
    echo "\$x = $x\n";
    $v = null;
  } while ($v !== null);
//  $x = $v % 3;	// rejected
*/
}

/* HH_FIXME[1002] call to main in strict*/
main();

