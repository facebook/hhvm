<?php
$s = "=?UTF-8?Q?=E2=82=AC?=";
$header = "$s\n $s\n\t$s";

var_dump(imap_mime_header_decode($header));