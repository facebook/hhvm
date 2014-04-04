<?php
$html = '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
</head>

<body>
<script type="text/javascript">alert("test")</script>
</body>
</html>';

$dom_document = new DOMDocument();
$dom_document->loadHTML($html);

$body_node = $dom_document->getElementsByTagName('body')->item(0);
$body_content = '';

foreach ($body_node->getElementsByTagName('script') as $node) {
  filter_dom_element($node);
}

foreach ($body_node->getElementsByTagName('style') as $node) {
  filter_dom_element($node);
}
exit;

function filter_dom_element($dom_element) {
  foreach ($dom_element->childNodes as $node) {
    var_dump(get_class($node));
  }
}
