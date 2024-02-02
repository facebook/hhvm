#!/usr/bin/env fbpython
import argparse
import io
import os
import subprocess
import sys

# Be careful here - normally python will look in the PWD first and that ends up being the 'lldb/' in this dir!
lldb_path = (
    subprocess.run(["/opt/llvm/bin/lldb", "-P"], stdout=subprocess.PIPE)
    .stdout.decode("utf-8")
    .strip()
)
sys.path.insert(0, lldb_path)
import lldb


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


parser = argparse.ArgumentParser(description="HHVM coredump data extractor")
parser.add_argument("hhvm_binary", help="the binary corresponding to the core file")
parser.add_argument("hhvm_core", help="the core file we want to extract data from")
parser.add_argument(
    "type",
    choices=["jitprof", "stacktrace"],
    help="the type of data we want to extract",
)
parser.add_argument("outfile", help="the file to store the extracted data in")
parser.add_argument("--hhvm_debuginfo", help="the debuginfo file associated with hhvm_binary")
args = parser.parse_args()

if not os.path.exists(args.hhvm_binary):
    raise RuntimeError("HHVM path not found")

if not os.path.exists(args.hhvm_core):
    raise RuntimeError("HHVM core not found")

if args.outfile == "-":
    args.outfile = "/dev/stdout"

debugger = lldb.SBDebugger.Create()
debugger.SetAsync(False)

# Attaching LLDB to HHVM tends to be really noisy. Redirect stderr temporarily.
with io.StringIO() as f:
    debugger.SetErrorFileHandle(f, False)
    eprint("Loading exe...")
    target = debugger.CreateTargetWithFileAndArch(
        args.hhvm_binary, lldb.LLDB_ARCH_DEFAULT
    )
    for line in f.getvalue().splitlines():
        if line.startswith(
            "error: hhvm Failed to extract location list table at offset"
        ):
            continue
        eprint(line)

eprint("Loading core...")
process = target.LoadCore(args.hhvm_core)

if not process.IsValid():
    raise RuntimeError("Unable to load core file")

if args.hhvm_debuginfo:
    ret = lldb.SBCommandReturnObject()
    debugger.GetCommandInterpreter().HandleCommand(
        f"target symbols add {args.hhvm_debuginfo}",
        ret
    )
    if not ret.Succeeded():
        raise RuntimeError(f"Unable to add debuginfo file: {ret.GetError()}")

def read_memory(addr, size):
    err = lldb.SBError()
    value = target.ReadMemory(lldb.SBAddress(addr, target), size, err)
    if err.Fail():
        raise RuntimeError(f"Error reading memory: {err}")
    return value

start_var = f"s_{args.type}_start"
start = target.FindFirstGlobalVariable(start_var)
if start.GetError().Fail():
    raise RuntimeError(f"Unable to access start variable '{start_var}': {start.GetError()}")
start_addr = int(start.GetValue(), 0)

end_var = f"s_{args.type}_end"
end = target.FindFirstGlobalVariable(end_var)
if end.GetError().Fail():
    raise RuntimeError(f"Unable to access end variable '{end_var}': {end.GetError()}")
end_addr = int(end.GetValue(), 0)

table_size = end_addr - start_addr
eprint(f"Found table at {start_addr} with size {table_size}")

with open(args.outfile, "wb") as f:
    memory = read_memory(start_addr, table_size)
    f.write(memory)

sys.exit(0)
