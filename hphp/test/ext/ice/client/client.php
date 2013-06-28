
<?php


function getProxy(){
	static $_G;
	if(!$_G ['ice_init']){
		require 'Ice.php';

		require 'class_demo.php';
		$_G ['ice_init'] = true;
	}

	$ICE = Ice_initialize();
	echo "create ice<br>\n";
	try{
	$p = $ICE->stringToProxy("classdemo:tcp -h 192.168.206.192 -p 10002:udp -p 10002");
	$hello = com_jd_CommontIceRpcServicePrxHelper::checkedCast($p);
	return $hello;

	}catch( Exception $ex){
		echo $ex;
	}
	return null;
}




function getIceFunction(){
	echo "getIceFunction====================<br>\n";
	$proxy=getProxy();
	try{
	$c=$proxy->getClassDemo();
       // var_dump( $c);
	echo $c;

	echo "newClassDemo==========================================\n";
	$d=$proxy->newClassDemo($c);

	var_dump($d);
	echo "getClassDemos============================================<br>\n";
	$e=$proxy->getClassDemoS();
		foreach ($e as $key => $value) {
			echo $value."<br>";
	//		var_dump($value);
		}

		$proxy->setClassDemos($e);
		$f=$proxy->getLongList();
		foreach ($f as $key => $value) {
			echo $value."<br>";
		}

		$proxy->setLongList($f);

	$g=$proxy->getOtherCommentById();
	}catch( Exception $ex){
       	//echo $ex;
	var_dump($ex);
        }

}


for($i=0;$i<20;$i++){
	echo "=====================".$i."<br>\n";
	getIceFunction();
	break;
}

?>

