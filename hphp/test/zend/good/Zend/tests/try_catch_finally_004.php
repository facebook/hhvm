<?php
function dummy($msg) {
   var_dump($msg);
}
try {
    try {
        var_dump("try");
        return;
    } catch (Exception $e) {
        dummy("catch");
        throw $e;
    } finally {
        dummy("finally");
    }
} catch (Exception $e) {
  dummy("catch2");
} finally {
  dummy("finally2");
}
var_dump("end");
?>