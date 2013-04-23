<?php
$text = "\xE3\x83\x86\xE3\x82\xB9\xE3\x83\x88\xE3\x83\x86\xE3\x82\xB9\xE3\x83\x88";
$options = array(
    'scheme' => 'Q',
    'input-charset' => 'UTF-8',
    'output-charset' => 'UTF-8',
    'line-length' => 30,
);

echo iconv_mime_encode('Subject', $text, $options);