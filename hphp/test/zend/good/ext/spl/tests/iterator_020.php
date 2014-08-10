<?php

echo "===Empty===\n";

$it = new AppendIterator;

foreach($it as $key=>$val)
{
	echo "$key=>$val\n";
}

echo "===Append===\n";

$it->append(new ArrayIterator(array(0 => 'A', 1 => 'B')));

foreach($it as $key=>$val)
{
	echo "$key=>$val\n";
}

echo "===Rewind===\n";

foreach($it as $key=>$val)
{
	echo "$key=>$val\n";
}

echo "===Append===\n";

$it->append(new ArrayIterator(array(2 => 'C', 3 => 'D')));

foreach(new NoRewindIterator($it) as $key=>$val)
{
	echo "$key=>$val\n";
}

echo "===Rewind===\n";

foreach($it as $key=>$val)
{
	echo "$key=>$val\n";
}

?>
===DONE===
<?php exit(0); ?>
