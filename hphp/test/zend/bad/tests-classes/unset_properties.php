<?php

class Test
{
	public    $publicProperty        = 'publicProperty set';

	protected $protectedProperty     = 'protectedProperty set';

	private   $privateProperty       = 'privateProperty set';
	
	public function __get($name)
	{
		return '__get "' . $name . '"';
	}
	
	public function __set($name, $value)
	{
		$this->$name = $value;
		echo '__set "' . $name . '" to "' . $value . '"';
	}
	
	public function __isset($name)
	{
		echo '__isset "' . $name . '"';
		return isset($this->$name);
	}
	
	public function getPublicProperty()
	{
		return $this->publicProperty;
	}
	
	public function setPublicProperty($publicProperty)
	{
		$this->publicProperty = $publicProperty;
	}

	public function unsetProtectedProperty()
	{
		unset($this->protectedProperty);
	}

	public function getProtectedProperty()
	{
		return $this->protectedProperty;
	}

	public function setProtectedProperty($protectedProperty)
	{
		$this->protectedProperty = $protectedProperty;
	}

	public function unsetPrivateProperty()
	{
		unset($this->privateProperty);
	}

	public function getPrivateProperty()
	{
		return $this->privateProperty;
	}

	public function setPrivateProperty($privateProperty)
	{
		$this->privateProperty = $privateProperty;
	}
}

// verifying public property
$o = new Test;
echo $o->publicProperty;
echo "\n";
var_export(isset($o->publicProperty));
echo "\n";
unset($o->publicProperty);
isset($o->publicProperty);
echo "\n";
echo $o->publicProperty;
echo "\n";
echo $o->getPublicProperty();
echo "\n";
echo $o->setPublicProperty('new publicProperty value via setter');
echo "\n";
echo $o->publicProperty;
echo "\n";
unset($o->publicProperty);
$o->publicProperty = 'new publicProperty value via public access';
echo "\n";
var_export(isset($o->publicProperty));
echo "\n";
echo $o->publicProperty;
echo "\n\n";

// verifying protected property
echo $o->getProtectedProperty();
echo "\n";
$o->unsetProtectedProperty();
var_export(isset($o->protectedProperty));
echo "\n";
echo $o->getProtectedProperty();
echo "\n";
echo $o->setProtectedProperty('new protectedProperty value via setter');
echo "\n";
var_export(isset($o->protectedProperty));
echo "\n";
echo $o->getProtectedProperty();
echo "\n\n";

// verifying private property
echo $o->getPrivateProperty();
echo "\n";
$o->unsetPrivateProperty();
var_export(isset($o->privateProperty));
echo "\n";
echo $o->getPrivateProperty();
echo "\n";
echo $o->setPrivateProperty('new privateProperty value via setter');
echo "\n";
var_export(isset($o->privateProperty));
echo "\n";
echo $o->getPrivateProperty();
echo "\n\n";

?>
