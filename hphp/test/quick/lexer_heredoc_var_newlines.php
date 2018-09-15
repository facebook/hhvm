<?hh
  $info = "hello";
echo <<<SCRIPT
<?hh
$info
throw new Exception(<<<TXT
$info

Fix the above and then run `arc build`
TXT
);
SCRIPT;
