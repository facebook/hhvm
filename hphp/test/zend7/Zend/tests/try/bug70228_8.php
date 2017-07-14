<?php

function foo() {
    $array = [1, 2, $n = 3];
    foreach ($array as $value) {
        var_dump($value);
        try {
            try {
            	switch (str_repeat("b", 2)) {
            		case "bb":
	                    return str_repeat("a", 2);
                }
            } finally {
                throw new Exception;
            }
        } catch (Exception $e) { }
    }
}

foo();
?>
===DONE===
