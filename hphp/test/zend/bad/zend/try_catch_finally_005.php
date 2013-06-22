<?php
for ($i = 0;  $i < 100 ; $i ++) {
    try {
        break;
    } finally {
        var_dump("break");
    }
}


for ($i = 0;  $i < 2; $i ++) {
    try {
        continue;
    } finally {
        var_dump("continue1");
    }
}

for ($i = 0;  $i < 3; $i ++) {
    try {
        try {
            continue;
        } finally {
            var_dump("continue2");
            if ($i == 1) {
                throw new Exception("continue exception");
            }
        }
    } catch (Exception $e) {
       var_dump("cactched");
    }  finally {
       var_dump("finally");
    }
}

?>