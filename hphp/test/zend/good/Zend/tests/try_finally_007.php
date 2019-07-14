<?hh
<<__EntryPoint>> function foo (): void {
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
