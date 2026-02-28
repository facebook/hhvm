<?hh

<<__EntryPoint>> function main(): void {
    require_once('test_base.inc');
    init();
    requestAll(vec[
        "test_get.php?name=Foo",
        "test_get.php?name=Bar",
        "subdoc//subdir/test.php",
        "subdoc/subdir/test.php",
    ]);
}
