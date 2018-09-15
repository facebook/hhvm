<?php


<<__EntryPoint>>
function main_printf_overrun() {
printf("%'", "");
}
