<?hh

function regular() :mixed{} // default coeffect

function rx($f)[rx] :mixed{ $f(); } // rx coeffect

<<__EntryPoint>>
function main() :mixed{
  // main has [defaults] coeffects and so lambda inherits [defaults].
  $f = () ==> { regular(); };
  // rx invokes lambda, [rx] attempts to call [defaults], which is a failure.
  rx($f);
}
