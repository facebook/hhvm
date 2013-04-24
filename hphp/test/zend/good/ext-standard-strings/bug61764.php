<?php
//expected -30000 mod 2^32 = 4294937296, and not -30000
//because we can represent 4294937296 with our PHP int type
print_r(unpack('I', pack('L', -30000)));