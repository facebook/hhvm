<?php
function a() {
    b();
}

function b() {
    c();
}

function c() {
    print_r(debug_backtrace(0, 1));
    print_r(debug_backtrace(0, 2));
    print_r(debug_backtrace(0, 0));
    print_r(debug_backtrace(0, 4));
}

a();
?>