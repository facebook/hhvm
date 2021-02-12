<?hh

function regular() {} // default coeffect

function rx($f)[rx] { $f(); } // rx coeffect

<<__EntryPoint>>
function main() {
  // main has [defaults] coeffects and so lambda inherits [defaults].
  $f = () ==> { regular(); };
  // rx invokes lambda, [rx] attempts to call [defaults], which is a failure.
  rx($f);
}
