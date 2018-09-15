# -*- mode: python -*-

load("@bazel_skylib//lib:paths.bzl", "paths")
load("@fbcode_macros//build_defs:compiler.bzl", "compiler")

include_defs("//hphp/DEFS.bzl")

def verify_unittest(suite, repo, dir, mode='interp,jit',
                    relocate=0, recycle_tc=0,
                    retranslate_all=0,
                    jit_serialize=0,
                    cli_server=0,
                    hhas_roundtrip=False, target_suffix='',
                    extra_args=[], blacklist=None,
                    noop_rule=False):

  # hphp_skip_repo_test and hphp_skip_non_repo_test let us enable or disable
  # tests based on repo mode. This is useful to shard our tests into different
  # sets for CI runs.
  if repo and native.read_config('hhvm', 'skip_repo_test'):
    noop_rule = True

  if not repo and native.read_config('hhvm', 'skip_non_repo_test'):
    noop_rule = True

  target_name = 'verify_' + suite + '_' + mode + \
       ('_repo' if repo else '') + \
       ('_relocate' if relocate else '') + \
       ('_retranslate-all' if retranslate_all else '') + \
       ('_jit-serialize' if jit_serialize else '') + \
       ('_recycle-tc' if recycle_tc else '') + \
       ('_cli-server' if cli_server else '') + \
       ('_hhas_roundtrip' if hhas_roundtrip else '') + \
       target_suffix

  target_name = target_name.replace('/', '_')
  target_name = target_name.replace(',', '_')

  generate_hhir_asserts = not is_opt_hhvm_build() and \
    compiler.get_compiler_for_current_buildfile() == 'clang'

  command = [
    '/usr/local/hphpi/bin/hhvm',
    '$(location //hphp/test:run.php)',
    suite,
    '-m',
    mode,
  ] + extra_args + (
    [ '-a', '-vEval.HHIRGenerateAsserts=true' ] if generate_hhir_asserts else
    []
  )
  if repo:
    command.extend(['-r',
                    '--repo-threads', '5'])
  if relocate != 0:
    command.extend(['--relocate', '%d' % relocate,
                    '--exclude-pattern', '=/debugger|ext_vsdebug/='])
  if retranslate_all != 0:
    command.extend(['--retranslate-all', '%d' % retranslate_all,
                    '--exclude-pattern', '=/debugger|ext_vsdebug/='])
  if jit_serialize != 0:
    command.extend(['--jit-serialize', '%d' % jit_serialize])
  if recycle_tc != 0:
    command.extend(['--recycle-tc', '%d' % recycle_tc,
                    '--exclude-pattern', '=/debugger|ext_vsdebug/='])
  if hhas_roundtrip:
    command.extend(['--hhas-round-trip',
                    '--exclude-pattern', '=/debugger|ext_vsdebug/='])

  if cli_server != 0:
    command.append('--cli-server')

  deplist=[
    '//hphp/hhvm:hhvm',
    # This dependency on hhvm_link shouldn't be necessary, since hhvm already
    # depends (indirectly) on hhvm_link. But we have it here for two reasons:
    #
    # 1) Our test targets need to be within 6 dependency hops of all of our
    #    source files (see https://fburl.com/3dpob2w2).
    #
    # 2) In a dev build, when Buck gets the hhvm binary from cache, it doesn't
    #    build all the necessary .so files (this is an issue with hhvm:hhvm
    #    being a custom_rule). A direct dependency on hhvm_link forces Buck to
    #    build them.
    '//hphp/hhvm:hhvm_link',
    '//hphp/hhvm:symlinks',
    '//hphp/runtime:runtime_core',
    '//hphp/facebook/extensions:facebook_extensions',
    ('' if dir.startswith('//') else '//hphp/test:') + dir,
  ]

  if blacklist != None:
      command.extend(['-x', blacklist])
      head = paths.dirname(blacklist)
      tail = paths.basename(blacklist)
      deplist.append('//' + head + ':' + tail)

  if noop_rule:
    custom_unittest(
      name=target_name,
      command=['true'],
      deps=deplist,
      type='simple',
    )
  else:
    custom_unittest(
      name=target_name,
      command=command,
      deps=deplist,
      tags=['hphp-test'] + (['run_as_bundle'] if cli_server != 0 else []),
    )
