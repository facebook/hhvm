<?php
function foo($ex = NULL) {
    try {
        try {
            goto label;
        } finally {
            var_dump("finally1");
            if ($ex) throw $ex;
        } 
    } catch (Exception $e) {
       var_dump("catched");
       if ($ex) return "return1";
    } finally {
       var_dump("finally2");
    }

label:
   var_dump("label");
   return "return2";
}

var_dump(foo());
var_dump(foo(new Exception()));

?>