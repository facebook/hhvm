<?hh
    //CURL_MULTI_GETCONTENT TEST

    //CREATE RESOURCES
     //$ch1=undefined;
    $ch2=curl_init();

    //SET URL AND OTHER OPTIONS
    try { curl_setopt($ch1, CURLOPT_URL, "file://".dirname(__FILE__). DIRECTORY_SEPARATOR . "curl_testdata1.txt"); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    curl_setopt($ch2, CURLOPT_URL, "file://".dirname(__FILE__). DIRECTORY_SEPARATOR . "curl_testdata2.txt");
    try { curl_setopt($ch1, CURLOPT_RETURNTRANSFER, true); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    curl_setopt($ch2, CURLOPT_RETURNTRANSFER, true);

    //CREATE MULTIPLE CURL HANDLE
    $mh=curl_multi_init();

    //ADD THE 2 HANDLES
    try { curl_multi_add_handle($mh,$ch1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    curl_multi_add_handle($mh,$ch2);

    //EXECUTE
    $running=0;
    do {
        curl_multi_exec($mh, inout $running);
    } while ($running>0);


    $results1 = null;
    try { $results1=curl_multi_getcontent($ch1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }  //incorrect parameter type
        $results2=curl_multi_getcontent($ch2);

    //CLOSE
     //curl_multi_remove_handle($mh,$ch1);
    curl_multi_remove_handle($mh,$ch2);
    curl_multi_close($mh);

    echo $results1;
    echo $results2;

