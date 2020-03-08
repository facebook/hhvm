<?hh // strict

namespace NS_nullable_and_arithmetic;

class C {
  public ?int $nint = 5;
}

function f(?int $p1, C $p2): void {
/*
  $p1++;		// flagged
  $p1--;		// flagged
  ++$p1;		// flagged
  --$p1;		// flagged
*/
/*
  $p2->nint++;	// not flagged
  $p2->nint--;	// not flagged
  ++$p2->nint;	// flagged
  --$p2->nint;	// flagged
*/
}

function main(): void {
  $v = null;
//  $v++;			// flagged
//  $q = $v + 3;		// flagged
//  $q = $v * 1.2;		// flagged
//  $q = $v >> 3;		// flagged

/*
// Typing error: This is a num (int/float) because this is used in an arithmetic operation
// It is incompatible with a nullable type
//
  $v = null;
  $v++;
  $v--;
  ++$v;
  --$v;
*/
  $c = new C();
/*
Typing error: This is a num (int/float) because this is used in an arithmetic operation
It is incompatible with a nullable type

  $v = $c->nint;
  $v++;
  $v--;
  ++$v;
  --$v;
/*
/*
  $v = $c->nint;
  if (!is_null($v)) {	// all OK!
    $v++;
    $v--;
    ++$v;
    --$v;
  } else {
//  $v++;		// flagged, as outside true path
  }

  if (is_int($v)) {	// all OK!
    $v++;
    $v--;
    ++$v;
    --$v;
  }

  if (is_float($v)) {	// all OK, but tests false for an ?int
    $v++;
    $v--;
    ++$v;
    --$v;
  }

// doesn't work, 'cos is_numeric includes numeric strings, and Hack doesn't allow arithmetic on strings
//
//  if (is_numeric($v)) {
//    $v++;
//    $v--;
//    ++$v;
//    --$v;
//  }
*/
///*
  $c->nint = 5;	// if not omitted, all 4 ops "pass the checker"
  $c->nint++;	// not flagged
  $c->nint--;	// not flagged
  ++$c->nint;	// flagged if assignment is omitted
  --$c->nint;	// flagged if assignment is omitted
//  var_dump($c);

  if (!is_null($c->nint)) {	// all OK!
    $c->nint++;
    $c->nint--;
    ++$c->nint;
    --$c->nint;
  }
//*/
}

/* HH_FIXME[1002] call to main in strict*/
main();
