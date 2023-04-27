<?hh

function foo(): void {
  $x = 0;

  $f = () ==> {
    // This does not shadow the local in the outer scope, as the outer
    // $x is not affected.
    foreach (vec[] as $x) {
    }
  };
}

// A second function to silence the naming lint about single definition files.
function bar(): void {}
