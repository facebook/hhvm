<?php
function test() {
    try {
        return str_repeat("a", 2);
    } finally {
        throw new Exception("ops");
    }
}

try {
    test();
} catch (Exception $e) {
    echo $e->getMessage(), "\n";
}
?>
