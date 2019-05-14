<?php <<__EntryPoint>> function main() {
echo mb_strimwidth('helloworld', 0, 5, '...', 'UTF-8') . "\n";
echo mb_strimwidth('hello', 0, 5, '...', 'UTF-8');
}
