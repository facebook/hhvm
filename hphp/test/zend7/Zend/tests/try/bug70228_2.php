<?php
function test() {
    try {
        throw new Exception(1);
    } finally {
        try {
            throw new Exception(2);
        } finally {
            return 42;
        }
    }
}

var_dump(test());
?>
