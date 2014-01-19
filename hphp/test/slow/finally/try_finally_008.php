<?php
function foo () {
    do {
        try {
            try {
            } finally {
                break;
            }
        } catch (Exception $e) {
        } finally {
        }
    } while (0);
}

foo();
?>
