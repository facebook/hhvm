<?php

$post_xml = '<?xml version="1.0" encoding="utf-8"?><ScanResults version="1.0"><scannedItem itemType="5" itemSize="1079856" itemName="C:\\Program Files\\VMware\\VMware Tools\\VMwareUser.exe" IsScanned="1" IsInfected="0" ObjectSummary="0" ScanError="0"/></ScanResults>';
$xml = new SimpleXMLElement($post_xml);
foreach ($xml->scannedItem as $item) {
  echo $item['itemName'] . "\n";
}
