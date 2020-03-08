<?hh // strict

namespace NS_noreturn;

class ExceptionA extends \Exception {}
class ExceptionB extends \Exception {}

// ============================== test functions ==============================

/*
function f1(): noreturn { }		// implicit return nothing is disallowed
function f2(): noreturn { return; }	// explicit return nothing is disallowed
function f3(): noreturn { return 10; }	// explicit return anything is disallowed

function f4(bool $p): noreturn { 
  if ($p) exit(10);
}					// implicit return nothing on false is disallowed

function f5(bool $p): noreturn { 
  if ($p) ; else exit(10);
}					// implicit return nothing on true is disallowed
*/

function f6(int $p): noreturn { 
  if ($p < 0) throw new ExceptionA();
  else if ($p > 0) throw new ExceptionB();
  else exit(10);
}					// Okay; no path returns

//function caller1(): void { $p = f6(123); }	// disallowed, as function does not return at all let along a value


// What about nested noreturn-function calls?

/*
This is disallowed on 2 fronts:
  Invalid return type (Typing[4110])
    This is noreturn (throws or exits)
    It is incompatible with void because this function implicitly returns void
    You are using the return value of a noreturn function (Typing[4133])
    A noreturn function always throws or exits
*/

/*
function f7a(int $p): noreturn { 
  var_dump(f6($p));			// disallowed; can't use non-existent value as arg to var_dump
}
*/

function f7b(int $p): noreturn { 
  return f6($p);				// Currently allowed; bug??
}

function f7c(int $p): noreturn { 
  f6($p);
}						// implicit return nothing is allowed

function f7d(int $p): noreturn { 
  f6($p);
//  return;					// explicit return nothing is disallowed
}

// ============================== test instance methods ==============================

class C1 {
/*
  public function f1(): noreturn { }		// implicit return nothing is disallowed
  public function f2(): noreturn { return; }	// explicit return nothing is disallowed
  public function f3(): noreturn { return 10; }	// explicit return anything is disallowed

  public function f4(bool $p): noreturn { 
    if ($p) exit(10);
  }						// implicit return nothing on false is disallowed

  public function f5(bool $p): noreturn { 
    if ($p) ; else exit(10);
  }						// implicit return nothing on true is disallowed
*/

  public function f6(int $p): noreturn { 
    if ($p < 0) throw new ExceptionA();
    else if ($p > 0) throw new ExceptionB();
    else exit(10);
  }						// Okay; no path returns but surpising. Not supposed to work here 
}

// ============================== test instance methods ==============================

class C2 {
/*
  public static function f1(): noreturn { }		// implicit return nothing is disallowed
  public static function f2(): noreturn { return; }	// explicit return nothing is disallowed
  public static function f3(): noreturn { return 10; }	// explicit return anything is disallowed

  public static function f4(bool $p): noreturn { 
    if ($p) exit(10);
  }							// implicit return nothing on false is disallowed

  public static function f5(bool $p): noreturn { 
    if ($p) ; else exit(10);
  }							// implicit return nothing on true is disallowed
*/

  public static function f6(int $p): noreturn { 
    if ($p < 0) throw new ExceptionA();
    else if ($p > 0) throw new ExceptionB();
    else exit(10);
  }							// Okay; no path returns
}

// ============================== end of script ==============================
