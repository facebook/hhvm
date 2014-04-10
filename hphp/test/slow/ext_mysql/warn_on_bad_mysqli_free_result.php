<?php

function main() {
  var_dump(mysqli_free_result(false));
  var_dump(@mysqli_free_result(false));
}

main();
