<?hh
class test {
    function hdlr($errno, $errstr, $errfile, $errline) {
        printf("[%d] errstr: %s, errfile: %s, errline: %d\n", $errno, $errstr, $errfile, $errline, $errstr);
    }
}
<<__EntryPoint>> function main(): void {
set_error_handler(varray[new test(), "hdlr"]);

trigger_error("test");
}
