<?hh
<<__EntryPoint>> function main(): void {
$streams = vec[
	'data://,;test',
	'data://text/plain,test',
	'data://text/plain;charset=US-ASCII,test',
	'data://;charset=UTF-8,Hello',
	'data://text/plain;charset=UTF-8,Hello',
	'data://,a,b',
	];

foreach($streams as $stream)
{
	var_dump(@file_get_contents($stream));
}

echo "===DONE===\n";
}
