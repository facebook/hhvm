<?hh

function __autoload($c)
{
  include 'bug26640.inc';
}

$a = new ReflectionClass('autoload_class');

if (is_object($a)) {
	echo "OK\n";
}

