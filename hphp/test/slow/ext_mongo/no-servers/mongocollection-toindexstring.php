<?php
class MyCollection extends MongoCollection
{
	static public function toIndexString($a)
	{
		return parent::toIndexString($a);
	}
}
var_dump(MyCollection::toIndexString('x'));
var_dump(MyCollection::toIndexString('x.y.z'));
var_dump(MyCollection::toIndexString('x_y.z'));
var_dump(MyCollection::toIndexString(array('x' => 1)));
var_dump(MyCollection::toIndexString(array('x' => -1)));
var_dump(MyCollection::toIndexString(array('x' => 1, 'y' => -1)));
?>
