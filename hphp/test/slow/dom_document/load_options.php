<?php
// saveHTML() should not add a default doctype when
// LIBXML_HTML_NODEFDTD is passed
$doc = new DOMDocument();
$doc->loadHTML("<html><body>test</body></html>", LIBXML_HTML_NODEFDTD);
echo $doc->saveHTML();
