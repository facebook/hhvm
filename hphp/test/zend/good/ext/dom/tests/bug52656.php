<?hh <<__EntryPoint>> function main(): void {
$CData = new DOMCdataSection('splithere!');
$CDataSplit = $CData->splitText(5);
echo get_class($CDataSplit), "\n";
var_dump($CDataSplit->data);
}
