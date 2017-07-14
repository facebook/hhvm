<?php

function gen() {
    try {
        throw new Exception(1);
    } finally {
        try {
            throw new Exception(2);
        } finally {
            try {
                yield;
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
