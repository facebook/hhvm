//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
type TShape = shape('x' => int);
type TInt = int;
class Foo {}

//// bar.php
<?hh
// package pkg1

// =====================================================================
// Section 1: lambda type annotations at top level (no if-package guard)
// =====================================================================

// Lambda parameter referring to pkg2 typedef from pkg1 - SHOULD ERROR
function lambda_param_top(): void {
  $f = (TShape $_) ==> 0;
}

// Lambda return type referring to pkg2 typedef from pkg1 - SHOULD ERROR
function lambda_return_top(): void {
  $f = (): TInt ==> 0;
}

// Lambda parameter referring to pkg2 class - separate Cross_pkg_access path
function lambda_class_top(): void {
  $f = (Foo $_) ==> 0;
}

// =====================================================================
// Section 2: lambda inside if (package pkg2) - the key case under test.
// pkg2 is now in loaded_packages, so the cross-package check should pass.
// =====================================================================

function lambda_param_in_if_package(): void {
  if (package pkg2) {
    // Lambda inside if (package pkg2) - should NOT error
    $f = (TShape $_) ==> 0;
  }
}

function lambda_return_in_if_package(): void {
  if (package pkg2) {
    $f = (): TInt ==> 0;
  }
}

function lambda_class_in_if_package(): void {
  if (package pkg2) {
    $f = (Foo $_) ==> 0;
  }
}

// =====================================================================
// Section 3: lambda after the if-package block - guard only covers body
// =====================================================================

function lambda_after_if_package(): void {
  if (package pkg2) {
    $a = (TShape $_) ==> 0; // ok inside
  }
  // pkg2 no longer loaded here - SHOULD ERROR
  $b = (TShape $_) ==> 0;
}

// =====================================================================
// Section 4: nested if-packages and `else` branch
// =====================================================================

function lambda_in_else(): void {
  if (package pkg2) {
    $ok = (TShape $_) ==> 0;
  } else {
    // pkg2 NOT loaded here - SHOULD ERROR
    $bad = (TShape $_) ==> 0;
  }
}

// =====================================================================
// Section 5: long-form Efun (function() { ... }) inside if (package pkg2)
// =====================================================================

function efun_in_if_package(): void {
  if (package pkg2) {
    $f = function(TShape $_): TInt {
      return 0;
    };
  }
}

// =====================================================================
// Section 6: closure type hints (function(T): T) - the *outer* hint
// is on a parameter; the inner T is closure-internal and explicitly
// not enforced. Test that even at top level, closure params/returns
// inside `(function(T): T)` don't fire.
// =====================================================================

function closure_hint_top(): void {
  // Inside the (function(TShape): TInt) the inner aliases are
  // not enforced. But the enclosing context is a local var; no
  // enforceable position involved. No error expected either way.
  $f = ((function(TShape): TInt) $g) ==> $g(shape('x' => 1));
  $f((TShape $_) ==> 0); // outer lambda has TShape -- top-level: SHOULD ERROR
}

// =====================================================================
// Section 7: lambda with generic-as referring to pkg2 typedef
// =====================================================================

function lambda_generic_as_top<T as TShape>(T $_): void {
  // The function's tparam constraint already errors at top level (signature).
  // The lambda body just uses the tparam; not relevant to the alias check.
}

function lambda_inside_generic_as(): void {
  if (package pkg2) {
    // Generic constraints on lambdas... lambdas don't take tparams in surface
    // syntax. Just verify nested lambda inside if-package still ok.
    $f = (TShape $a) ==> $a;
  }
}

// =====================================================================
// Section 8: lambda inside a __RequirePackage function - the per-fn
// attribute already grants pkg2 access for the function signature; it
// should also flow to lambdas defined in the body.
// =====================================================================

<<__RequirePackage("pkg2")>>
function lambda_in_require_package(): void {
  $f = (TShape $_) ==> 0;            // expected ok
  $g = function(): TInt { return 0; }; // expected ok
}

// =====================================================================
// Section 9: method body contains if (package pkg2) with a lambda
// =====================================================================

class HasMethods {
  public function with_if_package(): void {
    if (package pkg2) {
      $f = (TShape $_) ==> 0;        // expected ok
    }
    // pkg2 not loaded here
    $bad = (TShape $_) ==> 0;        // expected error
  }
}

// =====================================================================
// Section 10: nested if-package - inner block sees both packages.
// (pkg2 typedefs visible in either layer; this just confirms nesting
// composes and there's no regression)
// =====================================================================

function nested_if_package(): void {
  if (package pkg2) {
    if (package pkg3) {
      $f = (TShape $_) ==> 0;        // expected ok (pkg2 still loaded)
    }
    // Still inside pkg2 guard.
    $g = (TShape $_) ==> 0;          // expected ok
  }
}

// =====================================================================
// Section 11: deeply nested - lambda returning a lambda referring to
// a pkg2 typedef, all inside if (package pkg2). Confirms nested
// expression contexts still see the loaded packages.
// =====================================================================

function nested_lambdas_in_if_package(): void {
  if (package pkg2) {
    $outer = (): (function(TShape): TInt) ==> {
      return (TShape $_) ==> 0;       // inner lambda sig - expected ok
    };
  }
}
