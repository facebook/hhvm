<?php

function main() {
  var_dump(mysqli_free_result(false));
  var_dump(@mysqli_free_result(false));
}


<<__EntryPoint>>
function main_warn_on_bad_mysqli_free_result() {
main();
}
