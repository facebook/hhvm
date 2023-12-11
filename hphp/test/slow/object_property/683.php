<?hh


<<__EntryPoint>>
function main_683() :mixed{
$one = dict['cluster'=> 1, 'version'=>2];
var_dump(isset($one->cluster));
var_dump(!($one->cluster ?? false));
$two = 'hello';
var_dump(isset($two->scalar));
}
