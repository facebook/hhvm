<?hh

<<__EntryPoint>>
function foo () {
    try {
label:
        echo "label";
        try {
        } finally {
            goto label;
            echo "dummy";
        }
    } catch (Exception $e) {
    } finally {
    }
}
