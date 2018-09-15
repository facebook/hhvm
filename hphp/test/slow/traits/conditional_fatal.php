<?php


<<__EntryPoint>>
function main_conditional_fatal() {
$execute_fatal = false;
if ($execute_fatal) {
  class C1 {
    use NonExistent;
  }
}

echo 'Good', "\n";

$execute_fatal = true;
if ($execute_fatal) {
  class C2 {
    use NonExistent;
  }
}

echo 'Bad', "\n";
}
