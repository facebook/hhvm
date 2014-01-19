<?php

error_reporting(E_ALL);

session_id("abtest");
session_start();
?>
<form>
<fieldset>
<?php

ob_flush();

ini_set("url_rewriter.tags", "a=href,area=href,frame=src,input=src,form=");

?>
<form>
<fieldset>
<?php

ob_flush();

ini_set("url_rewriter.tags", "a=href,area=href,frame=src,input=src,form=fakeentry");

?>
<form>
<fieldset>
<?php

ob_flush();

ini_set("url_rewriter.tags", "a=href,fieldset=,area=href,frame=src,input=src");

?>
<form>
<fieldset>
<?php

session_destroy();
?>