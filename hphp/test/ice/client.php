
<?php
require 'Ice.php';
require 'class_demo.php';

$ICE = Ice_initialize();
    try
    {
	echo "======================";		
	   $p = $ICE->stringToProxy("classdemo:tcp -h 127.0.0.1 -p 10002");
	   $p->ice_timeout(1000);
	
      $hello = com_jd_CommontIceRpcServicePrxHelper::checkedCast($p);
		  var_dump($c);

      $c=$hello->getClassDemo();
			var_dump( $c);
			$d=$hello->newClassDemo($c);
			
			$e=$hello->getClassDemoS();
			foreach ($e as $key => $value) {
				echo $value."<br>";
			}
			echo "====================test|||||<br>\n";
			$hello->setClassDemos($e);
			$f=$hello->getLongList();
			foreach ($f as $key => $value) {
				echo $value."<br>";
			}

				$hello->setLongList($f);
            $g=$hello->getOtherCommentById();
			echo $g;
   } catch( Exception $ex)
    {
		echo "<br>======================";
	   echo $ex;
		echo "\n======================";
    }


?>

