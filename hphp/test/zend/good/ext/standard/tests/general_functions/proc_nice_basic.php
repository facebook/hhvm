<?php
	function getNice($id)
	{
		$res = shell_exec('ps -p ' . $id .' -o "pid,nice"');
		preg_match('/^\s*\w+\s+\w+\s*(\d+)\s+(\d+)/m', $res, $matches);
		if (count($matches) > 2)
			return $matches[2];
		else
			return -1;
	}
	$delta = 10;
	$pid = getmypid();
	$niceBefore = getNice($pid);
	proc_nice($delta);
	$niceAfter = getNice($pid);
	var_dump($niceBefore == ($niceAfter - $delta));
?>