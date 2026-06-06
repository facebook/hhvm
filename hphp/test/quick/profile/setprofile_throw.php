<?hh


function throwing_profiler($case, $func) :mixed{
  if ($case == 'enter' && ($func == 'bar' || $func == 'baz')) {
    throw new Exception("yeah");
  }
}

function bar() :mixed{ echo "bar()\n"; }
function baz() :mixed{ echo "baz()\n"; }

function foo($f) :mixed{
  $f();
}

<<__EntryPoint>>
function setprofile_throw(): void {
// Test throwing on function entry
  fb_setprofile('throwing_profiler');
  try { foo('bar'); } catch (Exception $x) { echo "Caught\n"; }
  try { foo('baz'); } catch (Exception $x) { echo "Caught\n"; }
  fb_setprofile(null);
}
