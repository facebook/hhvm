<?php

class Base
{
	public final function __construct()
	{
	}
}

class Works extends Base
{
}

class Extended extends Base
{
	public function Extended()
	{
	}
}

ReflectionClass::export('Extended');

?>