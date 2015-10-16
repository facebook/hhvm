<?hh

class SomeClass {}
function some_function() { new SomeClass(); }

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

echo "\nIMPLICIT CTORS\n";
fb_setprofile(($mode, $fn, $frame) ==> {
  echo "$mode $fn ".implode(',', array_keys($frame))."\n";
}, SETPROFILE_FLAGS_DEFAULT | SETPROFILE_FLAGS_CTORS);
some_function();
