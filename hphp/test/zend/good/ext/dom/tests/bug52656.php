<?php <<__EntryPoint>> function main() {
$CData = new DOMCdataSection('splithere!');
$CDataSplit = $CData->splitText(5);
echo get_class($CDataSplit), "\n";
var_dump($CDataSplit->data);
}
