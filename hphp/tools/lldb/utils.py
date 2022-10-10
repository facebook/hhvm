# Copyright 2022-present Facebook. All Rights Reserved

import abc
import argparse
import collections
import functools
import lldb
import re
import typing

class Command(abc.ABC):
        
    def __init__(self, debugger: lldb.SBDebugger, _internal_dict):
        self.parser = self.create_parser()
        self.help_string = self.parser.format_help()

    @classmethod
    def register_lldb_command(cls, debugger, module_name, top_module="") -> None:
        parser = cls.create_parser()
        cls.__doc__ = parser.format_help()
        command = f"command script add -o -c {top_module + '.' if top_module else ''}{module_name}.{cls.__name__} {cls.command}"
        debugger.HandleCommand(command)

    @classmethod
    def default_parser(cls) -> argparse.ArgumentParser:
        parser = argparse.ArgumentParser(
            description=cls.description,
            prog=cls.command,
            epilog=cls.epilog,
            formatter_class=argparse.RawDescriptionHelpFormatter,
        )
        return parser

    def get_short_help(self) -> str:
        return self.description

    def get_long_help(self) -> str:
        return self.help_string

    @classmethod
    @abc.abstractmethod
    def create_parser(cls) -> argparse.ArgumentParser:
        """ Create and return an ArgumentParser object.

            Typical usage is to call .add_argument()
            as needed on the parser that default_parser() returns,
            and return that parser.
        """
        ...

    @abc.abstractmethod
    def __call__(self, debugger: lldb.SBDebugger, command: str, exe_ctx: lldb.SBExecutionContext, result: lldb.SBCommandReturnObject):
        ...

    @property
    @abc.abstractmethod
    def command(self) -> str:
        """ The name used to call the command """
        ...

    @property
    @abc.abstractmethod
    def description(self) -> str:
        """ A short, one-line description of the command """
        ...

    @property
    def epilog(self) -> typing.Optional[str]:
        """ Additional information to display after the usage """
        return None


#------------------------------------------------------------------------------
# Memoization.

_all_caches = []


def memoized(func):
    """Simple memoization decorator that ignores **kwargs."""
    global _all_caches

    cache = {}
    _all_caches.append(cache)

    @functools.wraps(func)
    def memoizer(*args):
        if not isinstance(args, collections.Hashable):
            return func(*args)
        if args not in cache:
            cache[args] = func(*args)
        return cache[args]
    return memoizer


def destruct(t: str) -> str:
    return re.sub(r'^(struct|class|union)\s+', '', t)


def template_type(t: lldb.SBType) -> str:
    """Get the unparametrized name of a template type.
    
    For example, given 'HPHP::VMFixedVector<ObjectProps::quick_index>,
    return 'HPHP::VMFixedVector'
    """
    return destruct(t.GetUnqualifiedType().name.split('<')[0])


def rawptr(val: lldb.SBValue) -> typing.Optional[lldb.SBValue]:
    """Fully strip a smart pointer type to a raw pointer. References are re-cast as pointers. """

    def wrap(addr: int, ty: lldb.SBType):
        return val.CreateValueFromAddress("(tmp)", addr, ty).address_of

    if val.type.IsPointerType():
        return val
    elif val.type.IsReferenceType():
        return val.deref

    name = template_type(val.type)
    ptr = None

    if name == 'HPHP::CompactSizedPtr':
        ptr = rawptr(val.GetChildMemberWithName('m_data'))
    elif name == 'HPHP::CompactTaggedPtr':
        inner = val.type.GetTemplateArgumentType(0)
        ptr = wrap(val.GetChildMemberWithName('m_data').unsigned & 0xffffffffffff, inner)

    if ptr is None:
        print(f"(error) unable to strip pointer with value {val.value}")

    return ptr
