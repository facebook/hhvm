<?hh

function regular() :mixed{ } // defaults

function regular_writeprops()[write_props] :mixed{ }

function f($f)[rx] :mixed{ $f(); }

function test(int $times): void {
  switch ($times) {
    case 0:
      // here, the failure is in the lambda - it is `pure` and `pure` cannot
      // call defaults
      f(()[] ==> { regular(); });
      break;
    case 1:
      // here, f is `rx` and the lambda is also `rx`, but this fails because
      // the lambda can't call `defaults`
      f(()[rx] ==> { regular(); });
      break;
    case 2:
      // f is `rx` and the lambda inherits coeffects from `test` and is
      // therefore `defaults`, so when f (`rx`) attempts to call
      // `defaults`, it raises a violation
      f(() ==> { regular(); });
      break;
    case 3:
      // f is `rx`, lambda is `write_props`, so f cannot call lambda
      f(()[write_props] ==> { regular_writeprops(); });
      break;
    default:
      break;
  }
}

<<__EntryPoint>>
function main() :mixed{
  for ($ctr=0; $ctr < 4; ++$ctr) {
    test($ctr);
  }
}
