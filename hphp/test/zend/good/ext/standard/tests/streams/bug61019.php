<?hh
<<__EntryPoint>> function main(): void {
echo "Test\n";

$descriptorspec = dict[
	0 => vec["pipe", "r"],  // stdin is a pipe that the child will read from
	1 => vec["pipe", "w"],  // stdout is a pipe that the child will write to
	2 => vec["pipe", "w"]   // stderr is a pipe that the child will write to
];

$pipes = null;
$process=proc_open("echo testtext", $descriptorspec, inout $pipes);
if(is_resource($process))
{
	stream_set_blocking($pipes[0],false);
	stream_set_blocking($pipes[1],false);
	stream_set_blocking($pipes[2],false);
	stream_set_write_buffer($pipes[0],0);
	stream_set_read_buffer($pipes[1],0);
	stream_set_read_buffer($pipes[2],0);
	$stdin_stream="";
	$stderr_stream="";

	echo "External command executed\n";
	do
	{
		$process_state=proc_get_status($process);
		$tmp_stdin=stream_get_contents($pipes[1]);
		if($tmp_stdin)
		{
			$stdin_stream=$stdin_stream.$tmp_stdin;
		}
		$tmp_stderr=stream_get_contents($pipes[2]);
		if($tmp_stderr)
		{
			$stderr_stream=$stderr_stream.$tmp_stderr;
		}
	} while($process_state['running']);

	echo "External command exit: ".$process_state['exitcode']."\n";

	//read outstanding data
	$tmp_stdin=stream_get_contents($pipes[1]);
	if($tmp_stdin)
	{
		$stdin_stream=$stdin_stream.$tmp_stdin;
	}
	$tmp_stderr=stream_get_contents($pipes[2]);
	if($tmp_stderr)
	{
		$stderr_stream=$stderr_stream.$tmp_stderr;
	}

	fclose ($pipes[0]);
	fclose ($pipes[1]);
	fclose ($pipes[2]);

	proc_close($process);

	echo "stdout: ".$stdin_stream."\n";
	echo "stderr: ".$stderr_stream."\n";
}
else
{
	echo "Can't start external command\n";
}
echo "===DONE===\n";
}
