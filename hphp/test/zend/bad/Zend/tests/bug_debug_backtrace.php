<?php
function foo() {
    bar();
}
                                                                                                            
function bar() {
    boo();
}
                                                                                                            
function boo(){
    debug_print_backtrace();
}
                                                                                                            
eval("foo();");
                                                                                                            
echo "Done\n";
?>
===DONE===