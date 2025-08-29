import abc
from typing import List

import lldb


class SimpleAlias(abc.ABC):
    alias: str
    command: str

    @classmethod
    def register_lldb_alias(
        cls,
        debugger: lldb.SBDebugger,
    ) -> None:
        command = f"command alias {cls.alias} {cls.command}"
        debugger.HandleCommand(command)


# LLDB seems to have less-than-stellar support for the regular expressionses defined in
# substitutions, i.e. failing to understand \s or \S, hence the verbose regexes in some of the
# aliases below.
class RegexAlias(abc.ABC):
    alias: str
    substitutions: List[str]
    help: str
    syntax_help: str

    @classmethod
    def register_lldb_alias(
        cls,
        debugger: lldb.SBDebugger,
    ) -> None:
        command = f"command regex -h '{cls.help}' -s '{cls.syntax_help}' {cls.alias} {" ".join("'" + s + "'" for s in cls.substitutions)}"
        debugger.HandleCommand(command)


class Show(RegexAlias):
    alias = "show"
    substitutions = [
        "s/env(ironment)?/settings show target.env-vars/",
        "s/args/settings show target.run-args/",
    ]
    help = "Generic command for showing things about the debugger."
    syntax_help = "show {env(ironment) | args}"


# (Un?)fortunately 'set' is already an existing command in LLDB,
# so we will not alias "set env" or "set args".
class Unset(RegexAlias):
    alias = "unset"
    substitutions = [
        "s/env[ \t]+([a-zA-Z_]+[a-zA-Z0-9_]*)/settings remove target.env-vars %1/"
    ]
    help = 'Unset certain variables. Complement to some "settings set".'
    syntax_help = "unset env <setting-index | setting-key>"


class ThreadReturn(SimpleAlias):
    alias = "return"
    command = "thread return"


class Inspect(SimpleAlias):
    alias = "inspect"
    command = "print"


class Where(SimpleAlias):
    alias = "where"
    command = "bt"


class Backtrace(SimpleAlias):
    alias = "backtrace"
    command = "bt"


class Info(RegexAlias):
    alias = "info"
    substitutions = [
        "s/break/breakpoint list/",
        "s/threads/thread list/",
        "s/registers/register read/",
        "s/all-registers/register read --all/",
        r"s/proc[ \t]+mappings/memory region --all/",
        "s/shared/image list/",
    ]
    help = "Generic command for showing things about the program being debugged."
    syntax_help = (
        "info {break, threads, registers, all-registers, proc mappings, shared}"
    )


class DeleteBreakpoint(SimpleAlias):
    alias = "delete"
    command = "breakpoint delete"


class DisableBreakpoint(SimpleAlias):
    alias = "disable"
    command = "breakpoint disable"


class EnableBreakpoint(SimpleAlias):
    alias = "enable"
    command = "breakpoint enable"


class DumpMemory(RegexAlias):
    alias = "dump"
    # TODO add other dump subcommands
    substitutions = [
        "s/memory[ \t]+([^ \t]+)[ \t]+([^ \t]+)[ \t]+([ \t]+)/memory read --outfile %1 %2 %3/",
    ]
    help = "Dump target code/data to a local file"
    syntax_help = "dump memory <file> <start> <end>"


def __lldb_init_module(debugger: lldb.SBDebugger, _top_module: str = "") -> None:
    """Register the aliases in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name at module load time.

    Arguments:
        debugger: Current debugger object

    Returns:
        None
    """
    for cls in SimpleAlias.__subclasses__():
        cls.register_lldb_alias(debugger)
    for cls in RegexAlias.__subclasses__():
        cls.register_lldb_alias((debugger))
