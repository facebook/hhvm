<?hh
    // wrong parameter count
    $s_c = null;
    try { $s_c = socket_close(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    var_dump($s_c);
