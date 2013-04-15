<?php
  $info = "hello";
echo <<<SCRIPT
<?php
$info
throw new Exception(<<<TXT
$info

Fix the above
TXT
);
SCRIPT;
