import lldb
import shlex

try:
    # LLDB needs to load this outside of the usual Buck mechanism
    import idx
    import utils
except ModuleNotFoundError:
    import hhvm_lldb.idx as idx
    import hhvm_lldb.utils as utils


def lookup_func(func_id: lldb.SBValue) -> lldb.SBValue:
    """ Find the function corresponding to a given FuncID

    Args:
        func_id: A HPHP::FuncId wrapped in an lldb.SBValue

    Returns:
        func: A HPHP::Func* wrapped in an lldb.SBValue
    """
    target = func_id.target
    assert func_id.type.name == "HPHP::FuncId", f"invalid func_id, type given is {func_id.type.name} but expected HPHP::FuncId"
    func_vec = target.FindFirstGlobalVariable("HPHP::Func::s_funcVec")
    if func_vec.IsValid():
        # Non-LowPtr
        func_id_val = utils.get(func_id, "m_id").unsigned
        result = idx.atomic_low_ptr_vector_at(func_vec, func_id_val)
        assert result.IsValid(), "returned invalid HPHP::Func"
    else:
        # LowPtr
        result = utils.rawptr(utils.get(func_id, 'm_id'))

    func_ptr = result.Cast(utils.Type('HPHP::Func', target).GetPointerType())
    assert func_ptr.IsValid(), "couldn't return HPHP::Func *"
    return func_ptr


def lookup_func_from_frame_pointer(fp: lldb.SBValue) -> lldb.SBValue:
    """ Get the jitted function pointed to by the given frame pointer.

    Args:
        fp: Activation record (HPHP::ActRec)

    Returns:
        func: An SBValue representing a HPHP::Func*
    """
    func_id = utils.get(fp, 'm_funcId')
    return lookup_func(func_id)


class LookupCommand(utils.Command):
    command = "lookup"
    description = "Look up HHVM runtime objects by ID"

    class ArgsNamespace:  # noqa: B903
        # argparse will add attributes to this class
        def __init__(self, exe_ctx: lldb.SBExecutionContext, result: lldb.SBCommandReturnObject):
            self.exe_ctx = exe_ctx
            self.result = result

    @classmethod
    def create_parser(cls):
        parser = cls.default_parser()
        subparsers = parser.add_subparsers(title="List of lookup subcommands")

        func_cmd = subparsers.add_parser(
            "func",
            help="Look up a Func* by its FuncId",
        )
        func_cmd.add_argument(
            "funcid",
            help="A HPHP::FuncId (i.e. int) uniquely identifying a HPHP::Func*"
        )
        func_cmd.set_defaults(func=cls._lookup_func_prep)

        litstr_cmd = subparsers.add_parser(
            "litstr",
            help="Look up a litstr StringData* by its Id and Unit*",
            epilog="If no Unit is given, the current unit (set by `unit`) is used.",
        )
        litstr_cmd.add_argument(
            "id",
            help="The ID of the desired StringData (i.e. an HPHP::Id)",
        )
        litstr_cmd.add_argument(
            "unit",
            nargs="?",
            help="The unit to use",
        )
        litstr_cmd.set_defaults(func=cls._lookup_litstr_prep)

        return parser

    def __init__(self, debugger, internal_dict):
        super().__init__(debugger, internal_dict)

    def __call__(self, debugger, command, exe_ctx, result):
        namespace = self.ArgsNamespace(exe_ctx, result)
        command_args = shlex.split(command)
        try:
            options = self.parser.parse_args(command_args, namespace=namespace)
            options.func(options)
        except SystemExit:
            result.SetError("option parsing failed")
            return

    @classmethod
    def _lookup_func_prep(cls, options):
        func_id_type = utils.Type("HPHP::FuncId", options.exe_ctx.target)
        func_id = options.exe_ctx.frame.EvaluateExpression(options.funcid).Cast(func_id_type)
        res = lookup_func(func_id)
        if res is None:
            options.result.SetError(f"cannot get function identified with FuncId {func_id}")
            return

        options.result.write(str(res))

    @classmethod
    def _lookup_litstr_prep(cls, options):
        raise NotImplementedError

def __lldb_init_module(debugger, _internal_dict, top_module=""):
    """ Register the commands in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name at module load time.

    Arguments:
        debugger: Current debugger object
        _internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    LookupCommand.register_lldb_command(debugger, __name__, top_module)
