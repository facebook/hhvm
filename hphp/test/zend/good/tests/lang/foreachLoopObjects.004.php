<?hh

class C {
	public $a = "Original a";
	public $b = "Original b";
	public $c = "Original c";
	public $d = "Original d";
	public $e = "Original e";
}
<<__EntryPoint>>
function entrypoint_foreachLoopObjects_004(): void {

  echo "\nRemoving the current element from an iterated object.\n";
  $obj = new C;
  $count=0;
  foreach ($obj as $v) {
  	if ($v==$obj->b) {
  		unset($obj->b);
  	}
  	var_dump($v);
  	if (++$count>10) {
  		echo "Loop detected.\n";
  		break;
  	}
  }
  var_dump($obj);


  echo "===DONE===\n";
}
