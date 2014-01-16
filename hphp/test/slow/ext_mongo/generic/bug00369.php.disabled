<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();

class CursorWrapper implements Iterator
{
	protected $mongoCursor;

	public function __construct(MongoCursor $mongoCursor)
	{ $this->mongoCursor = $mongoCursor; }

	public function current()
	{ return $this->mongoCursor->current(); }

	public function key()
	{ return $this->mongoCursor->key(); }

	public function rewind()
	{ return $this->mongoCursor->rewind(); }

	public function next()
	{ return $this->mongoCursor->next(); }

	public function valid()
	{ return $this->mongoCursor->valid(); }
}

$mongo_db = $mongo->selectDB('phpunit');
$cursor = $mongo_db->getGridFS()->find()->limit(20);
$wrappedCursor = new CursorWrapper($cursor);
iterator_to_array($wrappedCursor);
echo "OK\n";
?>