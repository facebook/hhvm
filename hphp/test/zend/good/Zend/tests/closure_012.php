<?php <<__EntryPoint>> function main() {
$lambda = function () use ($i) {
    return ++$i;
};
$lambda();
$lambda();
var_dump($i);
}
