<?php

function a() {
		echo "hello";
			unregister_tick_function('a');
}

declare (ticks=1);
register_tick_function('a');

echo "Done\n";
?>