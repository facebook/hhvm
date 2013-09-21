<?php
$envelope["from"]= 'host@domain.com';
$envelope["return_path"]= 'host@domain.com';

$part1["type"]=TYPETEXT;
$part1["subtype"]="plain";
$part1["encoding"]=ENCQUOTEDPRINTABLE ;
$part1["charset"]='iso-8859-2';
$part1["contents.data"]=imap_8bit('asn řkl');

$body = array($part1);

echo imap_mail_compose($envelope, $body);
?>