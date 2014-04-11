<?php

function variadic_hinted_objects(stdClass ...$objects) {}

function variadic_hinted_scalars(int ...$objects) {}

function main() {
  variadic_hinted_scalars(1, 2, 3, 4);
  variadic_hinted_objects(
    new stdClass(), new stdClass(), new stdClass(), new stdClass());
}
main();
