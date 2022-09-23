# Copyright 2022-present Facebook. All Rights Reserved

import abc

class Command(abc.ABC):
    @classmethod
    def register_lldb_command(cls, debugger, module_name, top_module=""):
        parser = cls.create_options()
        cls.__doc__ = parser.format_help()
        command = f"command script add -o -c {top_module + '.' if top_module else ''}{module_name}.{cls.__name__} {cls.command}"
        debugger.HandleCommand(command)

    @classmethod
    @abc.abstractmethod
    def create_options(cls):
        ...

    @property
    @abc.abstractmethod
    def command(self):
        ...
