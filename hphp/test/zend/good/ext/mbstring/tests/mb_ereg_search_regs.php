<?php
    // homepage: 
    
    //$mb_str = "Алексей Федорович Карамазов был Алексей Федорович Карамазов был kyrillischer string string";
    //      = "Lorem ipsum dolor sit amet"
    
    mb_ereg_search_init("Алексей Федорович Карамазов был Алексей Федорович Карамазов был");
    
    
    
    $match= mb_ereg_search_regs("ов");
    var_dump($match);  
    
    
?>