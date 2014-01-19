<?php

$sxe = new SimpleXMLElement('<image-definition />');
$sxe->addChild('path', 'some/path/to/my.file');
$sxe->addChild('options');
$sxe->options->addChild('paddingbottom', 1);
var_dump((string)$sxe->path);
var_dump((string)$sxe->options->paddingbottom);
