<?php

function gen() {
    try {
        throw new Exception(1);
    } finally {
        try {
            yield;
        } finally {
            try {
                throw new Exception(2);
            } finally {
            }
        }
    }
}

try {
    gen()->rewind();
} catch (Exception $e) {
    echo $e, "\n";
}

?>
