<?php
apc_delete('test');
var_dump(apc_inc('test'));
var_dump(apc_dec('test'));
