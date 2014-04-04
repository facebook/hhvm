<?php
$dom = new \DOMDocument();
$dom->loadHTML('
<html>
  <form id="form-1" action="" method="POST">
  </form>
  <form id="form_2" action="" method="POST">
  </form>
</html>');
$form1 = $dom->getElementById('form_2');
$form2 = $dom->getElementById('form_2');
var_dump(spl_object_hash($form1) === spl_object_hash($form2));
