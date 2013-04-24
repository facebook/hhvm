<?php
$s = new SplFileObject( __FILE__ );
try {
    $s->setMaxLineLen(-1);
}
catch (DomainException $e) {
    echo 'DomainException thrown';
}

?>