<?php
class foo extends php_user_filter {
    function filter($in, $out, &$consumed, $closing) {}
    function onCreate() {}
    function onClose() {}
}
class bar extends php_user_filter {
    function filter($in, $out, &$consumed) {}
    function onCreate() {}
    function onClose() {}
}
?>