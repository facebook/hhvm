<?hh
class test {
    <<__DynamicallyCallable>> function hdlr($errno, $errstr, $errfile, $errline) :mixed{
        printf("[%d] errstr: %s, errfile: %s, errline: %d\n", $errno, $errstr, $errfile, $errline, $errstr);
    }
}
<<__EntryPoint>> function main(): void {
set_error_handler(vec[new test(), "hdlr"]);

trigger_error("test");
}
