<?php
$xml =<<<XML
<r>
  <p>Test</p>
  <o d='h'>
    <xx rr='info' />
    <yy rr='data' />
  </o>
</r>
XML;

$x = simplexml_load_string($xml);

var_dump(isset($x->p));
var_dump(isset($x->p->o));
var_dump(isset($x->o->yy));
var_dump(isset($x->o->zz));
var_dump(isset($x->o->text));
var_dump(isset($x->o->xx));
?>
===DONE===