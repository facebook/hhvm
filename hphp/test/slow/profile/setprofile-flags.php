<?hh

class SomeClass {
  function some_method() :mixed{}
}

<<__NEVER_INLINE>>
function some_function() :mixed{ new SomeClass(); }

<<__EntryPoint>>
function entrypoint_setprofileflags(): void {

  echo "DEFAULT\n";
  fb_setprofile(($mode, $fn, $frame) ==> {
    echo "$mode $fn ".implode(',', array_keys($frame))."\n";
  });
  some_function();

  echo "\nEXITS\n";
  fb_setprofile(($mode, $fn, $frame) ==> {
    echo "$mode $fn ".implode(',', array_keys($frame))."\n";
  }, SETPROFILE_FLAGS_EXITS);
  some_function();

  echo "\nENTERS\n";
  fb_setprofile(($mode, $fn, $frame) ==> {
    echo "$mode $fn ".implode(',', array_keys($frame))."\n";
  }, SETPROFILE_FLAGS_ENTERS);
  some_function();

  echo "\nFRAME PTRS\n";
  fb_setprofile(($mode, $fn, $frame) ==> {
    echo "$mode $fn ".implode(',', array_keys($frame))."\n";
  }, SETPROFILE_FLAGS_DEFAULT | SETPROFILE_FLAGS_FRAME_PTRS);
  some_function();

  echo "\nTHIS OBJECT\n";
  fb_setprofile(($mode, $fn, $frame) ==> {
    echo "$mode $fn ".implode(',', array_keys($frame))."\n";
  }, SETPROFILE_FLAGS_DEFAULT | SETPROFILE_FLAGS_FRAME_PTRS |
    SETPROFILE_FLAGS_THIS_OBJECT__MAY_BREAK);
  (new SomeClass())->some_method();

  echo "\nIMPLICIT CTORS\n";
  fb_setprofile(($mode, $fn, $frame) ==> {
    echo "$mode $fn ".implode(',', array_keys($frame))."\n";
  }, SETPROFILE_FLAGS_DEFAULT | SETPROFILE_FLAGS_CTORS);
  some_function();
}
