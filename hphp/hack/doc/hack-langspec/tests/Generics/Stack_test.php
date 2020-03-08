<?hh // strict

namespace NS_Stack_test;

require_once ('Stack.php');

function main2(\NS_Stack\Stack<int> $stInt, \NS_Stack\Stack<float> $stFloat): void {
  $stInt->push(10);
  $stInt->push(20);
  $stInt->push(30);
  echo 'pop => ' . $stInt->pop() . "\n";
//  $stInt->push(10.5);	// type argument float is incompatible with type parameter int

  $stFloat->push(10.12);
  $stFloat->push(-4320.6);
  echo 'pop => ' . $stFloat->pop() . "\n";
//  $stFloat->push(100);	// type argument int is incompatible with type parameter float
  echo 'pop => ' . $stFloat->pop() . "\n";

  echo 'pop => ' . $stInt->pop() . "\n";
  echo 'pop => ' . $stInt->pop() . "\n";
}

function process(\NS_Stack\Stack<int> $p1): void {}
//function process(\NS_Stack\Stack<float> $p1): void {}
//function process(\NS_Stack\Stack<num> $p1): void {}
//function process(\NS_Stack\Stack<mixed> $p1): void {}

function main(): \NS_Stack\Stack<num> {
  $stInt = new \NS_Stack\Stack();
  $stInt->push(100);
  echo 'pop => ' . $stInt->pop() . "\n";

  echo 'stack depth = ' . $stInt->getStackDepth() . "\n";
//  process($stInt);			// fixes the type as Stack<int>

// With the line above commented out
// Pushing a float or string on an intended int stack is allowed; need to discuss this under "type inferencing"
// It seems like, ultimately, the stack type really is Stack<Mixed>
// With the call to processInt enabled, push(float) and push(string) are rejected as being incompatible

  $stInt->push(10.5);		// rejected once stack type is fixed
  var_dump($stInt);
  echo 'pop => ' . $stInt->pop() . "\n";

//  process($stInt);			// locks in the type as Stack<num>

  $stInt->push('abc');		// rejected once stack type is fixed
  var_dump($stInt);
  echo 'pop => ' . $stInt->pop() . "\n";

//  process($stInt);			// fixes the type as Stack<mixed>



  $stInt = new \NS_Stack\Stack();
  $stInt->push(100);
  $stInt->push(10.5);
  return $stInt;		// fixes the type as Stack<num>
}

//main2(new \NS_Stack\Stack(), new \NS_Stack\Stack());
/* HH_FIXME[1002] call to main in strict*/
main();
