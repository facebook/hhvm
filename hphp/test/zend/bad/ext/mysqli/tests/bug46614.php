<?php
class MySQL_Ext extends mysqli{
	protected $fooData = array();

	public function isEmpty()
	{
		$this->extData[] = 'Bar';
		return empty($this->extData);
	}
}

include ("connect.inc");
$MySQL_Ext = new MySQL_Ext($host, $user, $passwd, $db, $port, $socket);

$isEmpty = $MySQL_Ext->isEmpty();
var_dump($isEmpty);
?>