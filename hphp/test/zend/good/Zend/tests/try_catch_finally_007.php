<?hh
function foo($ret = FALSE) {
    try {
        try {
            do {
                goto label;
            } while(0);
            foreach (varray[] as $val) {
                continue;
            }
        } finally {
            var_dump("finally1");
            throw new Exception("exception");
        }
    } catch (Exception $e) {
        goto local;
local:
        var_dump("catched");
        if ($ret) return "return";
    } finally {
       var_dump("finally2");
    }

label:
   var_dump("label");
}

<<__EntryPoint>> function main(): void {
var_dump(foo());
var_dump(foo(true));
}
