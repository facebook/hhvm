<?php

class Base
{
	public final function Base()
	{
	}
}

class Works extends Base
{
}

class Extended extends Base
{
	public function __construct()
	{
	}
}

ReflectionClass::export('Extended');

?>