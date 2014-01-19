<?php
$xml = new XMLWriter();
$xml->openMemory();
$xml->startDocument('1.0', 'UTF-8');
$xml->startElementNS(
  null, // null prefix
  'urlset',
  'http://www.sitemaps.org/schemas/sitemap/0.9'
);
$xml->endElement();
$xml->endDocument();
echo $xml->flush();

$xml = new XMLWriter();
$xml->openMemory();
$xml->startDocument('1.0', 'UTF-8');
$xml->startElementNS(
  '', // empty string prefix
  'urlset',
  'http://www.sitemaps.org/schemas/sitemap/0.9'
);
$xml->endElement();
$xml->endDocument();
echo $xml->flush();


$xml = new XMLWriter();
$xml->openMemory();
$xml->startDocument('1.0', 'UTF-8');
$xml->startElementNS(
  'foo', // non-empty string prefix
  'urlset',
  'http://www.sitemaps.org/schemas/sitemap/0.9'
);
$xml->endElement();
$xml->endDocument();
echo $xml->flush();
