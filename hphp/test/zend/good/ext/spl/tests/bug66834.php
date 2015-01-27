<?php

// overrides both offsetExists and offsetGet
class ArrayObjectBoth extends ArrayObject 
{
	public function offsetExists($offset) {
		var_dump('Called: '.__METHOD__);
		return parent::offsetExists($offset);
	}

	public function offsetGet($offset) {
		var_dump('Called: '.__METHOD__);
		return parent::offsetGet($offset);
	}
}

// overrides only offsetExists
class ArrayObjectExists extends ArrayObject 
{
	public function offsetExists($offset) {
		var_dump('Called: '.__METHOD__);
		return parent::offsetExists($offset);
	}
}

// overrides only offsetGet
class ArrayObjectGet extends ArrayObject 
{
	public function offsetGet($offset) {
		var_dump('Called: '.__METHOD__);
		return parent::offsetGet($offset);
	}
}

// overrides only offsetGet and offsetSet
class ArrayObjectGetSet extends ArrayObject
{
	public function offsetGet($offset)
	{
		return parent::offsetGet(str_rot13($offset));
	}

	public function offsetSet($offset, $value)
	{
		return parent::offsetSet(str_rot13($offset), $value);
	}
}

$values = ['foo' => '', 'bar' => null, 'baz' => 42];

echo "==== class with offsetExists() and offsetGet() ====\n";
$object = new ArrayObjectBoth($values);
var_dump($object->offsetExists('foo'), isset($object['foo']), empty($object['foo']));
var_dump($object->offsetExists('bar'), isset($object['bar']), empty($object['bar']));
var_dump($object->offsetexists('baz'), isset($object['baz']), empty($object['baz']));
var_dump($object->offsetexists('qux'), isset($object['qux']), empty($object['qux']));

echo "==== class with offsetExists() ====\n";
$object = new ArrayObjectExists($values);
var_dump($object->offsetExists('foo'), isset($object['foo']), empty($object['foo']));
var_dump($object->offsetExists('bar'), isset($object['bar']), empty($object['bar']));
var_dump($object->offsetexists('baz'), isset($object['baz']), empty($object['baz']));
var_dump($object->offsetexists('qux'), isset($object['qux']), empty($object['qux']));

echo "==== class with offsetGet() ====\n";
$object = new ArrayObjectGet($values);
var_dump($object->offsetExists('foo'), isset($object['foo']), empty($object['foo']));
var_dump($object->offsetExists('bar'), isset($object['bar']), empty($object['bar']));
var_dump($object->offsetexists('baz'), isset($object['baz']), empty($object['baz']));
var_dump($object->offsetexists('qux'), isset($object['qux']), empty($object['qux']));

echo "==== class with offsetGet() and offsetSet() ====\n";
$object = new ArrayObjectGetSet;
$object['foo'] = 42;
var_dump($object->offsetExists('foo'), $object->offsetExists('sbb'), isset($object['foo']), isset($object['sbb']), empty($object['sbb']));

?>
