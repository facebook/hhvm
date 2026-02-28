#!/bin/sh

# Expect $3 to be the path to hackc, and $4 the path to hackfmt
$3 $1 $2 | $4
