<?php

const HERP_DERP = 'Foo Bar';

// This is not output in repo-authoritative mode due to constant propagation
var_dump(__FILE__);
