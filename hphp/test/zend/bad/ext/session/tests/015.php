<?php
error_reporting(E_ALL);

session_id("abtest");
session_start();
?>
<a href="/link?<?php echo SID; ?>">
<?php
session_destroy();
?>