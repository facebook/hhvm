<?php 
setlocale (LC_CTYPE, "C");
$sc_encoded = htmlspecialchars ("<>\"&��\n",ENT_COMPAT,"ISO-8859-1");
echo $sc_encoded;
$ent_encoded = htmlentities ("<>\"&��\n",ENT_COMPAT,"ISO-8859-1");
echo $ent_encoded;
echo html_entity_decode($sc_encoded,ENT_COMPAT,"ISO-8859-1");
echo html_entity_decode($ent_encoded,ENT_COMPAT,"ISO-8859-1");
?>