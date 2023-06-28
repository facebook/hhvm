<?hh


<<__EntryPoint>>
function main_conditional_fatal() :mixed{
$execute_fatal = false;
if ($execute_fatal) {
  include 'conditional_fatal-1.inc';
}

echo 'Good', "\n";

$execute_fatal = true;
if ($execute_fatal) {
  include 'conditional_fatal-2.inc';
}

echo 'Bad', "\n";
}
