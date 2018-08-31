<?hh
<<__Memoize>>
function test_top_level() { static $i = 100; return $i++; }


<<__EntryPoint>>
function main_toplevel() {
echo test_top_level().' ';
echo test_top_level();
}
