#! /bin/bash
#

/usr/lib/rpm/perl.prov $* |
sed -e '/perl(hostnames)/d' -e '/perl(lib::mtr.*/d' -e '/perl(lib::v1.*/d' -e '/perl(mtr_.*/d' -e '/perl(My::.*/d'

