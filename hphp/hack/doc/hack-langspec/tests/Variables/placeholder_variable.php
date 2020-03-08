<?hh // strict

namespace NS_placeholder_variable;

function f(mixed $p): void {
  var_dump($p);

  echo "\$_ is >" . (string)$p . "<\n";
  $_ = 12.345;
}

function g(int $_): void {	// checker accepts $_ as a parameter name, but probably shouldn't
  echo "\$_ is >" . $_ . "<\n";
}

class C {
  public float $_ = 0.0;	// checker accepts $_ as an instance field name, but probably shouldn't
}

trait T {
  public float $_ = 0.0;	// checker accepts $_ as an instance field name, but probably shouldn't
}

function main(): void {

  g(33);	// Surprise! Outpus "$_ is >33<"

  echo "\n\n========================= Using \$_ as an exception catch parameter name =========================\n\n";

  try {
  }
  catch (\Exception $_) {	// checker accepts $_ here, but probably shouldn't
  }

  echo "\n\n========================= Is \$_ defined by default? =========================\n\n";

//  var_dump($_);	// You are using the return value of a void function (Typing[4119])
			// A void function doesn't return a value ($_ is a placeholder variable not meant to be used)
//  echo "\$_ is >" . $_ . "<\n";	// runtime Notice: Undefined variable: _

//  f($_);	// You are using the return value of a void function (Typing[4119])
		// A void function doesn't return a value ($_ is a placeholder variable not meant to be used)

// The messages are confusing: one talks about a void function (at check time), the other about a non-existent 
// variable (at run time)

  echo "\n\n========================= Can the value of \$_ be changed? =========================\n\n";

  $_ = "Hello";		// allows me to change $_
//  var_dump($_);	// You are using the return value of a void function (Typing[4119])
			// A void function doesn't return a value ($_ is a placeholder variable not meant to be used)
  echo "\$_ is >" . $_ . "<\n";	// checks and outputs "$_ is >Hello<"

//  f($_);	// You are using the return value of a void function (Typing[4119])
		// A void function doesn't return a value ($_ is a placeholder variable not meant to be used)

  $_ = 10;		// allows me to change $_
  echo "\$_ is >" . $_ . "<\n";	// checks and outputs "$_ is >Hello<"
//  ++$_;		// This is a num (int/float) because this is used in an arithmetic operation
		// It is incompatible with void ($_ is a placeholder variable not meant to be used)
//  $_ += 5;	// This is a num (int/float) because this is used in an arithmetic operation
		// It is incompatible with void ($_ is a placeholder variable not meant to be used)
//  $v = $_ + 5;	// checker rejects ... void function and num/float

// echo usage is allowed by can't pass to a function or use on some other contexts

  echo "\n\n========================= Odd behavior with var_dump and \$_ =========================\n\n";

  $a = array('a' => 10, 'f' => 30);
  foreach ($a as $key => $value) {
    var_dump($key, $value, $_);	// checks and works, as expected; 'a', 10, "hello", and 'f', 30, "hello"
//    var_dump($_);	// You are using the return value of a void function (Typing[4119])
			// A void function doesn't return a value ($_ is a placeholder variable not meant to be used)
  }

// in this context, can use $_ in var_dump, but NOT on its own!!

  echo "\n\n========================= An intended use of \$_ with foreach, but ... =========================\n\n";

  foreach ($a as $key => $_) {
    var_dump($key, $_);
    $_ = 'xx';		// allows me to change $_
  }

//  var_dump($_);	// You are using the return value of a void function (Typing[4119])
			// A void function doesn't return a value ($_ is a placeholder variable not meant to be used)
  echo "\$_ is >" . $_ . "<\n";	// checks and works, as expected; $_ is >xx<

  echo "\n\n========================= An odd use of \$_ =========================\n\n";

  foreach ($a as $_) {
//    var_dump($_);	// You are using the return value of a void function (Typing[4119])
    echo "\$_ is >" . $_ . "<\n";	// checks and works, as expected; $_ is >10<; $_ is >30<

  }

//  var_dump($_);	// You are using the return value of a void function (Typing[4119])
			// A void function doesn't return a value ($_ is a placeholder variable not meant to be used)
  echo "\$_ is >" . $_ . "<\n";	// checks and works, as expected; $_ is >30<

  echo "\n\n========================= Using \$_ for both key and value =========================\n\n";

  foreach ($a as $_ => $_) {
//    var_dump($_);	// You are using the return value of a void function (Typing[4119])
    echo "\$_ is >" . $_ . "<\n";	// checks and works, outputting $_ is >a<; $_ is >f<
  }

// shows that key wins over value, but should this be guaranteed?

//  var_dump($_);	// You are using the return value of a void function (Typing[4119])
			// A void function doesn't return a value ($_ is a placeholder variable not meant to be used)
  echo "\$_ is >" . $_ . "<\n";	// checks and works, as expected; $_ is >f<

  echo "\n\n========================= An intended use of \$_ with list =========================\n\n";

  $v = Vector {1, 2, 3};
  list($a, $_, $c) = $v;	// checks and works, as expected
  var_dump($a, $c);		// outputs int(1); int(3)
  list($_, $b, $_) = $v;	// checks and works, as expected
  var_dump($b);			// outputs int(2)
}

/* HH_FIXME[1002] call to main in strict*/
main();
