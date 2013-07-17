<?php

function test() {
  list($a, $b) = yield wait_forva(result(1), result(2));
}
