<?php
$$A += $$B->a = &$$C; 
unset($$A);
$$A -= $$B->a = &$$C; 
unset($$A);
$$A *= $$B->a = &$$C; 
unset($$A);
$$A /= $$B->a = &$$C; 
unset($$A);
$$A **= $$B->a = &$$C; 
var_dump($$A);
?>
