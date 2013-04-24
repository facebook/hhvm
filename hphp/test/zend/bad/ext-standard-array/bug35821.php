<?php

class Element
{
	public function ThrowException ()
	{
		throw new Exception();
	}

	public static function CallBack(Element $elem)
	{
		$elem->ThrowException();
	}
}

$arr = array(new Element(), new Element(), new Element());
array_map(array('Element', 'CallBack'), $arr);

echo "Done\n";
?>