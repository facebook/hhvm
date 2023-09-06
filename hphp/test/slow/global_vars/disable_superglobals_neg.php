<?hh

<<__EntryPoint>>
function main() :mixed{
  $_GET;
  $_ = readonly $_ENV;
  list($_ENV, $x) = tuple(1, 2);
  foreach(vec[] as $_GET) {}
  // bar(inout $_SERVER); - fails for additional reason
  $_REQUEST["bar"] = "baz";
  $_FILES = "foobar";
  $_POST['bananas'] = "exists";
  $_POST['bananas'];
  idx($_POST, 'bananas');
  echo "No Errors\n";
}

// function bar(inout mixed $x) {}
