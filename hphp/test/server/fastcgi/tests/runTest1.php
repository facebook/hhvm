<?hh

require_once('test_base.inc');
<<__EntryPoint>> function main(): void {
requestAll(varray[
    "test_get.php?name=Foo",
    "test_get.php?name=Bar",
    "subdoc//subdir/test.php",
    "subdoc/subdir/test.php",
]);
}
