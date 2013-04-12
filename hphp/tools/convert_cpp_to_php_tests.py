#!/bin/env python

from glob import glob
import re
import os
import shutil

def main():
    prefix = 'runtime/tmp/TestCodeRunVM/Test'
    for path in glob(prefix + '*'):
        suite = camel_to_underscore(path.replace(prefix, ''))
        suite_path = 'test/tcr/' + suite + '/'
        mkdir_p(suite_path)
        for test in glob(path + '/*'):
            if not 'tcr-' in test:
                continue
            # I don't have a better name
            name = re.sub('.*tcr-', '', test)
            file_prefix = suite_path + name
            print file_prefix

            for f in glob(file_prefix + '*'):
                if os.path.isdir(f):
                    shutil.rmtree(f)
                else:
                    os.remove(f)

            file(file_prefix + '.php', 'w').write(
                re.sub(r'# hphp/test/test_code_run.cpp:\d+', '', file(test + '/main.php').read())
            )
            shutil.copyfile(test + '/test.exp', file_prefix + '.php.expect')
            if os.path.exists(test + '/test.opts'):
                opts = file(test + '/test.opts').read().strip()
                if opts == '-vEval.EnableHipHopSyntax=1':
                    os.symlink('../hiphop.opts', file_prefix + '.php.opts')
                elif opts == '-vEval.EnableXHP=1':
                    os.symlink('../xhp.opts', file_prefix + '.php.opts')
                elif opts == '-vEval.JitEnableRenameFunction=true':
                    os.symlink('../rename_functions.opts', file_prefix + '.php.opts')
                elif opts == '-vServer.APC.AllowObject=1':
                    os.symlink('../allow_object.opts', file_prefix + '.php.opts')
                elif opts == '-vServer.APC.AllowObject=0':
                    os.symlink('../disallow_object.opts', file_prefix + '.php.opts')
                elif opts == '-vEval.EnableHipHopSyntax=1\n-vEval.JitEnableRenameFunction=true':
                    os.symlink('../hiphop_and_rename.opts', file_prefix + '.php.opts')
                else:
                    raise Exception("unknown opt %s" % repr(opts))

            if os.path.exists(test + '/build.opts'):
                opts = file(test + '/build.opts').read().strip()
                if opts == '-vEnableHipHopSyntax=1':
                    os.symlink('../hiphop.hphp_opts', file_prefix + '.php.hphp_opts')
                elif opts == '-vEnableXHP=1':
                    os.symlink('../xhp.hphp_opts', file_prefix + '.php.hphp_opts')
                elif opts == '-vCopyProp=1\n-vEnableHipHopSyntax=1':
                    os.symlink('../hiphop_and_copy_prop.hphp_opts', file_prefix + '.php.hphp_opts')
                elif opts == '-vAllDynamic=1':
                    os.symlink('../all_dynamic.hphp_opts', file_prefix + '.php.hphp_opts')
                elif opts == '-vAllDynamic=1\n-v DynamicMethodPrefix.*=_':
                    os.symlink('../all_dynamic_and_dynamic_method.hphp_opts', file_prefix + '.php.hphp_opts')
                elif opts == '-vLocalCopyProp=0 -vEliminateDeadCode=0':
                    os.symlink('../copy_prop_and_no_dead_code.hphp_opts', file_prefix + '.php.hphp_opts')
                elif opts == '-vAutoInline=5':
                    os.symlink('../auto_inline_5.hphp_opts', file_prefix + '.php.hphp_opts')
                elif opts == '-vAutoInline=5\n-vEnableHipHopSyntax=1':
                    os.symlink('../hiphop_and_auto_inline_5.hphp_opts', file_prefix + '.php.hphp_opts')
                elif opts == '-vEnableHipHopSyntax=1\n-vArrayAccessIdempotent=1':
                    os.symlink('../hiphop_and_array_access_idempotent.hphp_opts', file_prefix + '.php.hphp_opts')
                elif opts == '-vDynamicInvokeFunctions.*=test1 -vDynamicInvokeFunctions.*=test2':
                    os.symlink('../dynamic_invoke_functions_test_1_and_2.hphp_opts', file_prefix + '.php.hphp_opts')
                else:
                    raise Exception("unknown hphp opt %s" % repr(opts))

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        pass

def camel_to_underscore(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

main()
