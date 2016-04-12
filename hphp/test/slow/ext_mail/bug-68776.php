<?php

/* patched HHVM will issue a warning about double mailheader */

mail("test@example.com", "Subject", "Message", "Header-1:1\n\nHeader-2");
