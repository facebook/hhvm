<?php
    $str = "中国abc + abc ?!？！字符＃　china string";

    $reg = "\w+";

    mb_regex_encoding("UTF-8");

    mb_ereg_search_init($str, $reg);
    $r = mb_ereg_search();

    if(!$r)
    {
        echo "null\n";
    }
    else
    {
        $r = mb_ereg_search_getregs(); //get first result
        do
        {
            var_dump($r[0]);
            $r = mb_ereg_search_regs();//get next result
        }
        while($r);
    }
?>