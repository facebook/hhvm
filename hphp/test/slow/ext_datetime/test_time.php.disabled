<?php
// XXX This test fails between 12:00AM and 12:59AM on 10/31/11 - 11/6/11
// and between 11:00PM and 11:59PM on 3/6/11 - 3/12/11, we should fix this.
// This issue appears to have something to do with how strtotime() handles
// daylight savings time.
$nextWeek = time() + (7 * 24 * 60 * 60);
var_dump(date("Y-m-d", $nextWeek) ==
         date("Y-m-d", strtotime("+1 week")));
