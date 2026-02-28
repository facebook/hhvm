//// b.php
<?hh
<<file: __PackageOverride('pkg2')>>

function pkg2_call(): void {}

//// c.php
<?hh
<<file: __PackageOverride('pkg3')>>
function pkg3_call(): void {}

//// a.php
<?hh
// package pkg1

// A higher-order function in pkg1
function apply_fn((function(): void) $fn): void {
  $fn();
}

function pkg1_call(): void {}

class Receiver {
  <<__SoftRequirePackage("pkg2")>>
  public function invokedInPkg2((function(): void) $fn): void {
    $fn();
  }

  <<__SoftRequirePackage('pkg3')>>
  public function invokedInPkg3((function(): void) $fn): void {
    $fn();
  }
}

class MyHandler {
  <<__SoftRequirePackage("pkg2")>>
  public function handleQuery(): void {
    // Direct call — error: __SoftRequirePackage does NOT allow calling functions in pkg2
    pkg2_call(); // error

    // Lambda — also error: __SoftRequirePackage does NOT load pkg2
    $f = () ==> {
      pkg2_call(); // error
    };
    $f();

    // Lambda passed to a method that also has __SoftRequirePackage("pkg2") — still error
    $receiver = new Receiver();
    $receiver->invokedInPkg2(() ==> {
      pkg2_call(); // error
    });

    // Lambda passed to a HOF (in pkg1, available everywhere) — still error for pkg2_call
    apply_fn(() ==> {
      pkg2_call(); // error
    });

    //  HOF with lambda calling pkg1 function (always available)
    apply_fn(() ==> {
      pkg1_call();
    });
  }

  // Method WITHOUT __SoftRequirePackage — lambdas here should NOT have pkg2
  public function handleWithoutPackage(): void {
    // Lambda calling pkg2 without package loaded — error
    $f = () ==> {
      pkg2_call(); // error
    };

    // Even passing to a method with __SoftRequirePackage doesn't help;
    $receiver = new Receiver();
    $receiver->invokedInPkg2(() ==> {
      pkg2_call(); // error
    });

    // HOF with lambda calling pkg1 — always fine
    apply_fn(() ==> {
      pkg1_call();
    });
  }

  <<__SoftRequirePackage('pkg3')>>
  public function handleMixedPackages(): void {
    $receiver = new Receiver();

    // A package 3 "chain" — still error: __SoftRequirePackage does NOT grant access to pkg2
    $receiver->invokedInPkg3(() ==> {
      pkg2_call(); // error
    });
  }

  <<__SoftRequirePackage('pkg2')>>
  public function handleMixedPackages2(): void {
    $receiver = new Receiver();

    // A package 2 should not be able to call anything package 3
    $receiver->invokedInPkg3(() ==> {
      pkg3_call(); // error
    });

    // should reject pkg3 calls in a lambda
    $receiver->invokedInPkg2(() ==> {
      pkg3_call(); // error
    });
  }
}

<<__SoftRequirePackage("pkg2")>>
function top_level_with_lambda(): void {
  // Lambda passed to HOF — error: __SoftRequirePackage does NOT allow calling functions in pkg2
  apply_fn(() ==> {
    pkg2_call(); // error
  });

}

<<__SoftRequirePackage("pkg3")>>
function top_level_with_lambda_subpackage(): void {
  // Lambda passed to a HOF — error: __SoftRequirePackage does NOT allow calling functions in pkg2
  apply_fn(() ==> {
    pkg2_call(); // error
  });

  // Calling a pkg1 function in a lambda — always works
  apply_fn(() ==> {
    pkg1_call();
  });
}
