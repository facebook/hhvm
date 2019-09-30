#!/bin/bash

# if we run this script with "buck test ..."", it would hang forever
# without NO_BUCKD, since buck daemon can do only one thing at a time
NO_BUCKD=1 buck run //hphp/hack/src:generate_full_fidelity

if [[ $(hg st) ]]; then
    echo "Found uncommited files after running generate_full_fidelity"
    echo "Please do \"buck run //hphp/hack/src:generate_full_fidelity\""
    echo "and include modified files in your commit"
    exit 1
fi
