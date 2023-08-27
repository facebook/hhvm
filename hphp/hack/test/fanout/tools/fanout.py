#!/usr/bin/env python3
# pyre-strict
from __future__ import annotations

import argparse
import logging
import os
import tempfile
from enum import Enum

import attr

from .fanout_test_driver import (
    Binaries,
    run_scenario_incremental_no_old_decls,
    run_scenario_incremental_with_old_decls,
    run_scenario_saved_state_init,
)
from .fanout_test_parser import FanoutTest

logging.basicConfig(
    format="[%(asctime)s] [%(levelname)s] %(message)s",
    datefmt="%m/%d/%Y %H:%M:%S %Z",
    level=logging.WARNING,
)


class Mode(Enum):
    SAVED_STATE_INIT = "saved-state-init"
    INCREMENTAL_OLD_DECLS_ENABLED = "incremental-old-decls-enabled"
    INCREMENTAL_OLD_DECLS_DISABLED = "incremental-old-decls-disabled"

    def __str__(self) -> str:
        return self.value


@attr.s(auto_attribs=True)
class Opts:
    hh_client: str
    hh_server: str
    hh_single_type_check: str
    legacy_hh_fanout: str
    debug: bool
    mode: Mode
    input_file: str

    def to_bins(self) -> Binaries:
        return Binaries(
            hh_client=self.hh_client,
            hh_server=self.hh_server,
            legacy_hh_fanout=self.legacy_hh_fanout,
            hh_single_type_check=self.hh_single_type_check,
        )


def get_temporary_dir(prefix: str) -> tempfile.TemporaryDirectory[str]:
    # sandcastle sets TEMP as a directory that's cleaned up when the job ends
    return tempfile.TemporaryDirectory(prefix=prefix, dir=os.getenv("TEMP"))


def go(opts: Opts) -> None:
    logging.debug("hh_client: %s", opts.hh_client)
    logging.debug("hh_server: %s", opts.hh_server)
    logging.debug("hh_single_type_check: %s", opts.hh_single_type_check)
    logging.debug("legacy_hh_fanout: %s", opts.legacy_hh_fanout)
    logging.debug("mode: %s", opts.mode)
    logging.debug("input_file: %s", opts.input_file)

    test = FanoutTest.from_file(opts.input_file)
    if opts.mode is Mode.SAVED_STATE_INIT:
        run_scenario_saved_state_init(opts.to_bins(), test)
    elif opts.mode is Mode.INCREMENTAL_OLD_DECLS_ENABLED:
        run_scenario_incremental_with_old_decls(opts.to_bins(), test)
    elif opts.mode is Mode.INCREMENTAL_OLD_DECLS_DISABLED:
        run_scenario_incremental_no_old_decls(opts.to_bins(), test)
    else:
        raise AssertionError()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--hh-client", type=os.path.abspath)
    parser.add_argument("--hh-server", type=os.path.abspath)
    parser.add_argument("--hh-single-type-check", type=os.path.abspath)
    parser.add_argument("--legacy-hh-fanout", type=os.path.abspath)
    parser.add_argument("--debug", action="store_true")
    parser.add_argument(
        "--mode", type=Mode, choices=list(Mode), default=Mode.SAVED_STATE_INIT
    )
    parser.add_argument("input_file")

    args = parser.parse_args()
    opts = Opts(**vars(args))

    if opts.debug:
        logging.getLogger().setLevel(level=logging.DEBUG)
    go(opts)


if __name__ == "__main__":
    main()
