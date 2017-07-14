<?php
try  {
        $a = "1";
        [1, (y().$a.$a) . ($a.$a)];
} catch (Error $e) {
        var_dump($e->getMessage());
}
?>
