#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

# To be sourced by main script

# Shamelessly copied from
# https://github.com/facebook/infer/blob/master/scripts/opam_utils.sh
# Many thanks to the infer team :D

opam_require_version_2 () {
    local status=0
    local version=0
    { version=$(opam --version 2>/dev/null); status=$?; }
    if [ "$status" != 0 ]; then
        # Suppress Warning: the `` without quotes in the next line is intentional
        # shellcheck disable=SC2016
        printf '*** ERROR: `opam --version` failed, please install opam version 2\n' >&2
        env >&2
        exit 1
    fi
    case $version in
        2*) ;;
        *)
            printf '*** ERROR: opam version "%s" is not supported, please install opam version 2\n' "$version" >&2
            printf '*** NOTE: opam is "%s"\n' "$(command -v opam)" >&2
            env >&2
            exit 1
    esac
}
