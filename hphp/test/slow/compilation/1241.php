<?php

interface A {
}
 class B implements A {
}
 class C extends B implements A {
}

 <<__EntryPoint>>
function main_1241() {
$obj = new C();
}
