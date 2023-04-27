#! /bin/bash
#

/usr/lib/rpm/perl.req $* |
sed -e '/perl(GD)/d' -e '/perl(hostnames)/d' -e '/perl(lib::mtr.*/d' -e '/perl(lib::v1.*/d' -e '/perl(mtr_.*/d' -e '/perl(My::.*/d'

