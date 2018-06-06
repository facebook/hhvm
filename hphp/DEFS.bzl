with allow_unsafe_import():
    import subprocess
    import platform as platmod

load("@fbcode_macros//build_defs:platform.bzl", "platform")

def get_fbcode_platform():
    return platform.get_platform_for_base_path(get_base_path())

def get_compiler_type(platform):
    return read_config("fbcode", "compiler_family")

def is_opt_hhvm_build():
    buck_out = read_config('project', 'buck_out')
    return '/opt' in buck_out

def is_dev_hhvm_build():
    return '/dev' in read_config('project', 'buck_out')

def is_lto_build(platform):
    return read_config("fbcode", "lto_type", "") != ""

def find_systemlib_files(dir, skip):
    find_cmd = [ 'find', dir, '-type', 'f', '-name', 'ext_*.php' ]
    proc = subprocess.Popen(find_cmd, stdout=subprocess.PIPE)
    files = str(proc.stdout.read().decode('ascii')).split('\n')
    return sorted(map(lambda x: x[len(dir) if skip else 0:],
                      filter(lambda x: x[:len(dir)] == dir, files)))

def get_platform_machine():
    return platmod.machine()
