<?hh

abstract final class ObjectMethod {
    public static $trace;
}

class W {
    function f($a) {
        ObjectMethod::$trace .= $a;
        return $a;
    }
}

<<__EntryPoint>>
function main() {
    $w = new W();
    ObjectMethod::$trace = "a";
    call_user_func_array(array($w, 'f'), array("b"));
    echo ObjectMethod::$trace . "\n";
    call_user_func_array(array($w, 'f'), array("c"));
    echo ObjectMethod::$trace . "\n";
}
