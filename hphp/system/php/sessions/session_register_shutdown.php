<?php

function session_register_shutdown() {
  register_shutdown_function('session_write_close');
}
