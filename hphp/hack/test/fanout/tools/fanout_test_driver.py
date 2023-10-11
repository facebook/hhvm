# pyre-strict
from __future__ import annotations

import json
import logging
import os
import re
import shutil
import subprocess
import sys
import tempfile
from typing import Any, Dict, List, Optional, Tuple

import attr

from .fanout_information import FanoutInformation
from .fanout_test_parser import FanoutTest

# pyre-fixme[24]: Generic type `re.Pattern` expects 1 type parameter.
WHITESPACE_SPLITTER: re.Pattern = re.compile(r"\s+")

DEFAULT_HH_SERVER_FLAGS: List[str] = [
    "--config",
    "hg_aware=false",
    "--config",
    "remote_type_check_enabled=false",
    "--config",
    "use_dummy_informant=true",
    "--config",
    "experiments_config_enabled=false",
    "--config",
    "symbolindex_search_provider=NoIndex",
    "--config",
    "num_local_workers=1",
    "--config",
    "max_workers=1",
    "--config",
    "allow_unstable_features=true",
    "--config",
    "allow_all_files_for_module_declarations=true",
    "--config",
    "disable_xhp_element_mangling=false",
    "--config",
    "ss_force=prod_with_flag_on:optimized_member_fanout",
]


@attr.s(auto_attribs=True)
class Binaries:
    hh_client: str
    hh_server: str
    hh_single_type_check: str
    legacy_hh_fanout: str

    def validate(self) -> None:
        if os.path.join(os.path.dirname(self.hh_client), "hh_server") != self.hh_server:
            # We don't have a way to specify the executable hh_client should use, and OCaml
            # resolves symlinks, so hh_server is not in the same directory as hh_client.
            # We does have to use PATH
            raise ValueError("{} must be an executable called hh_server")

    def exec_hh(
        self,
        args: List[str],
        allow_type_errors: bool = True,
        env: Optional[Dict[str, str]] = None,
        check: bool = True,
        **kwargs: Any,
    ) -> subprocess.CompletedProcess[str]:
        if env is None:
            env = {}
        prev_path = env.get("PATH", ":" + (os.getenv("PATH") or ""))
        env["PATH"] = os.path.dirname(self.hh_server) + prev_path
        if "HH_TEST_MODE" not in env:
            env["HH_TEST_MODE"] = "true"
        r = _exec([self.hh_client] + args, env=env, check=False, **kwargs)
        if allow_type_errors and r.returncode == 2:
            return r
        if check:
            r.check_returncode()
        return r

    def exec_hh_stop(self, repo_root: str) -> subprocess.CompletedProcess[str]:
        return self.exec_hh(["stop", repo_root])

    def exec_hh_single_type_check(
        self, args: List[str], **kwargs: Any
    ) -> subprocess.CompletedProcess[str]:
        return _exec([self.hh_single_type_check] + args, **kwargs)

    def exec_legacy_hh_fanout(
        self, args: List[str], **kwargs: Any
    ) -> subprocess.CompletedProcess[str]:
        return _exec([self.legacy_hh_fanout] + args, **kwargs)


@attr.s(auto_attribs=True)
class RepoRoot:
    path: str

    def cleanup(self) -> None:
        shutil.rmtree(self.path)

    def hhconfig_file(self) -> str:
        return os.path.join(self.path, ".hhconfig")


@attr.s(auto_attribs=True)
class SavedStateDir:
    path: str

    def cleanup(self) -> None:
        shutil.rmtree(self.path)

    def edges_dir(self) -> str:
        return os.path.join(self.path, "edges")

    def naming_table_blob_file(self) -> str:
        return os.path.join(self.path, "hh_mini")

    def depgraph_file(self) -> str:
        return os.path.join(self.path, "hh_mini.hhdg")

    def naming_table_sqlite_file(self) -> str:
        return os.path.join(self.path, "hh_naming.sql")

    def saved_state_spec(self, changed_files: List[str]) -> str:
        return json.dumps(
            {
                "data_dump": {
                    "deptable": self.depgraph_file(),
                    "state": self.naming_table_blob_file(),
                    "changes": changed_files,
                    "prechecked_changes": [],
                    "corresponding_base_revision": "-1",
                }
            }
        )


@attr.s(auto_attribs=True)
class ExecResult:
    exit_code: int
    stdout: str
    stderr: str


def _exec(
    args: List[str],
    capture_output: bool = True,
    timeout: int = 300,
    text: bool = True,
    check: bool = True,
    **kwargs: Any,
) -> subprocess.CompletedProcess[str]:
    logging.debug("_exec: run: %s (%s)", args, kwargs)
    try:
        v = subprocess.run(
            args,
            capture_output=capture_output,
            timeout=timeout,
            text=text,
            check=check,
            **kwargs,
        )
    except subprocess.CalledProcessError as e:
        logging.debug(
            "_exec: error (%d):\n=== STDOUT ===\n%s\n=== STDERR ===\n%s\n",
            e.returncode,
            e.stdout,
            e.stderr,
        )
        raise

    logging.debug("_exec: result: %s", repr(v))
    return v


def _create_temporary_directory(category: str, filename: str) -> str:
    # use TEMP on Sandcastle, this is autocleaned
    return tempfile.mkdtemp(
        prefix="{}-{}-".format(os.path.basename(filename), category),
        dir=os.getenv("TEMP"),
    )


def _prepare_repo_root(test: FanoutTest) -> RepoRoot:
    repo_root = RepoRoot(_create_temporary_directory("repo", test.filename))
    logging.debug("Preparing repo root in %s", repo_root.path)
    os.mknod(repo_root.hhconfig_file())
    test.prepare_base_php_contents(repo_root.path)
    return repo_root


def _make_repo_change(repo_root: RepoRoot, test: FanoutTest) -> List[str]:
    logging.debug("Updating repo root at %s", repo_root.path)
    return test.prepare_changed_php_contents(repo_root.path)


def _create_saved_state(
    bins: Binaries, repo_root: RepoRoot, test: FanoutTest
) -> Tuple[subprocess.CompletedProcess[str], SavedStateDir]:
    saved_state_dir = SavedStateDir(_create_temporary_directory("ss", test.filename))
    logging.debug(
        "Generating saved-state for %s in %s", repo_root.path, saved_state_dir.path
    )

    logging.debug("Step 1/3: Generating edges to %s", saved_state_dir.edges_dir())
    _exec(["mkdir", "-p", saved_state_dir.edges_dir()])
    bins.exec_hh(
        [
            "--no-load",
            "--save-64bit",
            saved_state_dir.edges_dir(),
            "--save-state",
            saved_state_dir.naming_table_blob_file(),
            "--gen-saved-ignore-type-errors",
            "--error-format",
            "raw",
            "--config",
            "store_decls_in_saved_state=true",
        ]
        + DEFAULT_HH_SERVER_FLAGS
        + [
            repo_root.path,
        ]
    )
    hh_result = bins.exec_hh(["--error-format", "raw", repo_root.path])
    logging.debug(
        "Step 2/3: Writing naming table to %s",
        saved_state_dir.naming_table_sqlite_file(),
    )
    bins.exec_hh(
        [
            "--save-naming",
            saved_state_dir.naming_table_sqlite_file(),
            repo_root.path,
        ]
    )
    bins.exec_hh_stop(repo_root.path)
    logging.debug(
        "Step 3/3: Building dependency graph to %s", saved_state_dir.depgraph_file()
    )
    bins.exec_legacy_hh_fanout(
        [
            "build",
            "--edges-dir",
            saved_state_dir.edges_dir(),
            "--output",
            saved_state_dir.depgraph_file(),
        ]
    )
    _exec(["rm", "-rf", saved_state_dir.edges_dir()])
    return (hh_result, saved_state_dir)


def _build_fanout_hash_map(bins: Binaries, test: FanoutTest) -> Dict[str, str]:
    m = {}
    r = bins.exec_hh_single_type_check(
        ["--dump-dep-hashes", "--no-builtins", test.filename]
    )
    for line in r.stdout.splitlines():
        line = line.strip()
        symbol_hash, symbol_name = WHITESPACE_SPLITTER.split(line, 1)
        m[symbol_hash] = symbol_name
    return m


def _launch_hh_from_saved_state(
    bins: Binaries,
    repo_root: RepoRoot,
    saved_state_dir: SavedStateDir,
    changed_files: List[str],
) -> subprocess.CompletedProcess[str]:
    logging.debug("Launching hh from saved-state for %s", repo_root.path)
    hh_result = bins.exec_hh(
        [
            "--config",
            "use_mini_state=true",
            "--config",
            "lazy_decl=true",
            "--config",
            "lazy_init2=true",
            "--config",
            "lazy_parse=true",
            "--with-mini-state",
            saved_state_dir.saved_state_spec(changed_files),
            "--config",
            "naming_sqlite_path={}".format(saved_state_dir.naming_table_sqlite_file()),
            "--config",
            "naming_sqlite_path={}".format(saved_state_dir.naming_table_sqlite_file()),
            "--config",
            "enable_naming_table_fallback=true",
            "--config",
            "log_categories=fanout_tests",
            "--error-format",
            "raw",
        ]
        + DEFAULT_HH_SERVER_FLAGS
        + [
            repo_root.path,
        ]
    )
    return hh_result


def _extract_fanout_information(
    bins: Binaries, repo_root: RepoRoot, tags: List[str]
) -> List[FanoutInformation]:
    server_log_file = bins.exec_hh(["--logname", repo_root.path]).stdout.strip()
    logging.debug("Extracting fanout information from %s", server_log_file)
    fis = FanoutInformation.extract_from_log_file(server_log_file)
    return [fi for fi in fis if fi.tag in tags]


def _strip_repo_root_from_output(repo_root: str, output: str) -> str:
    if repo_root[-1:] != os.sep:
        repo_root += os.sep
    return output.replace(repo_root, "")


def _format_result(
    fanout_information: List[FanoutInformation],
    fanout_hash_map: Dict[str, str],
) -> None:
    symbols = []
    for fi in fanout_information:
        symbols += [fanout_hash_map.get(h, h) for h in fi.hashes]
    symbols.sort()
    for s in symbols:
        print(s)


def run_scenario_saved_state_init(bins: Binaries, test: FanoutTest) -> None:
    """Run the saved-state init fanout scenario.

    This scenario involves a saved-state init with some local changes. It
    includes the following steps:

    1. Build a saved-state for the base version
    2. Kill hack
    3. Make the repo changes
    4. Initialize from the saved-state
    5. Extract saved-state fanout
    """
    repo_root = _prepare_repo_root(test)
    fanout_hash_map = _build_fanout_hash_map(bins, test)

    (hh_result_base, saved_state_dir) = _create_saved_state(bins, repo_root, test)
    changed_files = _make_repo_change(repo_root, test)
    _launch_hh_from_saved_state(
        bins,
        repo_root,
        saved_state_dir,
        changed_files=changed_files,
    )
    bins.exec_hh_stop(repo_root.path)

    fanout_information = _extract_fanout_information(
        bins, repo_root, tags=["saved_state_init_fanout"]
    )

    _format_result(
        fanout_information=fanout_information,
        fanout_hash_map=fanout_hash_map,
    )

    repo_root.cleanup()
    saved_state_dir.cleanup()


def run_scenario_incremental_no_old_decls(bins: Binaries, test: FanoutTest) -> None:
    """Run the incremental fanout scenario with old decls unavailable.

    This scenario involves calculating the fanout in an incremental change
    scenario (i.e. after saved-state initialization was successful), but where
    the old versions of the declarations are unavailable and thus fine-grained
    decl diffing is impossible.

    1. Build a saved-state for the base version
    2. Kill hack
    3. Initialize from the saved-state, but disabled cached decl loading
    4. Make the change
    5. Type check and extract fanout
    """
    repo_root = _prepare_repo_root(test)
    fanout_hash_map = _build_fanout_hash_map(bins, test)
    (hh_result_base, saved_state_dir) = _create_saved_state(bins, repo_root, test)

    _launch_hh_from_saved_state(
        bins,
        repo_root,
        saved_state_dir,
        changed_files=[],
    )
    _make_repo_change(repo_root, test)
    bins.exec_hh(["--error-format", "raw", repo_root.path])
    bins.exec_hh_stop(repo_root.path)

    fanout_information = _extract_fanout_information(
        bins, repo_root, tags=["incremental_fanout"]
    )

    _format_result(
        fanout_information=fanout_information,
        fanout_hash_map=fanout_hash_map,
    )

    repo_root.cleanup()
    saved_state_dir.cleanup()


def run_scenario_incremental_with_old_decls(bins: Binaries, test: FanoutTest) -> None:
    """Run the incremental fanout scenario with old decls available.

    This scenario involves calculating the fanout in an incremental change
    scenario (i.e. after saved-state initialization was successful), and where
    the old versions of the declarations are available and thus fine-grained
    decl diffing is possible.

    1. Build a saved-state for the base version
    2. Kill hack
    3. Initialize from the saved-state, forcing a re-typecheck of all files to
       make sure all decls are present in shared memory.
    4. Make the change
    5. Type check and extract fanout
    """
    repo_root = _prepare_repo_root(test)
    fanout_hash_map = _build_fanout_hash_map(bins, test)
    (hh_result_base, saved_state_dir) = _create_saved_state(bins, repo_root, test)

    _launch_hh_from_saved_state(
        bins,
        repo_root,
        saved_state_dir,
        changed_files=test.all_base_php_files(),
    )
    _make_repo_change(repo_root, test)
    bins.exec_hh(["--error-format", "raw", repo_root.path])
    bins.exec_hh_stop(repo_root.path)

    fanout_information = _extract_fanout_information(
        bins, repo_root, tags=["incremental_fanout"]
    )

    _format_result(
        fanout_information=fanout_information,
        fanout_hash_map=fanout_hash_map,
    )

    repo_root.cleanup()
    saved_state_dir.cleanup()
