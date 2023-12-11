<?hh


<<__EntryPoint>>
function main_no_double_slash() :mixed{
$streams = vec[
  'data:,A%20brief%20note',
  'data:application/vnd-xxx-query,select_vcount,fcol_from_fieldtable/local',
  'data:;base64,Zm9vYmFyIGZvb2Jhcg==',
  'data:,;test',
  'data:text/plain,test',
  'data:text/plain;charset=US-ASCII,test',
  'data:;charset=UTF-8,Hello',
  'data:text/plain;charset=UTF-8,Hello',
  'data:,a,b',
];

foreach ($streams as $stream)
{
  var_dump(@file_get_contents($stream));
}
}
