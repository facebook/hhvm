<?php
$xml =
'<?xml version="1.0" encoding="UTF-8"?>
<entry xmlns="http://www.w3.org/2005/Atom"
  xmlns:media="http://search.yahoo.com/mrss/"
  xmlns:yt="http://gdata.youtube.com/schemas/2007">

  <id>tag:youtube.com,2008:video:kgZRZmEc9j4</id>
  <yt:accessControl action="comment" permission="allowed"/>
  <yt:accessControl action="videoRespond" permission="moderated"/>
  <media:group>
    <media:title type="plain">Chordates - CrashCourse Biology #24</media:title>
    <yt:aspectRatio>widescreen</yt:aspectRatio>
  </media:group>
  <media:category label="Music" scheme="http://gdata.youtube.com/schemas/2007/categories.cat">Music</media:category>
</entry>';

$doc = new \DomDocument('1.0', 'UTF-8');
$doc->loadXML($xml);

$dxp = new \DomXPath($doc);
$query='(//namespace::*[name()="yt"])[last()]';
$out = $dxp->query($query);

print sizeof($out)."\n";
$item = $out->item(0);
print $item->nodeName."\n";
print $item->nodeValue."\n";
print $item->nodeType."\n";
print $item->prefix."\n";
print $item->localName."\n";
print $item->namespaceURI."\n";
