<?php
abstract class Test
{
	public static function get()
	{
		static::$a ?? true;
	}
}
Test::get();
?>
