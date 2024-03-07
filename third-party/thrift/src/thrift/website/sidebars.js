/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @format
 */

/** ABOUT SIDEBARS
 * This sidebar contains custom TOC structure for pages under the `doc` folder
 * It's intended to provide browsing experiences for external users (aka: outside of Meta)
 * To update sidebars related to the internally facing TOC (aka: Meta Users), see
 * `website/fb/sidebars.js` file
 */

const {
  fbContent,
  fbInternalOnly,
} = require('docusaurus-plugin-internaldocs-fb/internal');

module.exports = {
  ghSidebar: [
    'intro',
    ...fbInternalOnly([
      'fb/quick-start',
      {
        type: 'category',
        label: 'How To',
        link: {
          type: 'doc',
          id: 'fb/howtos/index',
        },
        items: [
          'fb/howtos/using-a-client',
          {
            type: 'category',
            label: 'Implementing a Server',
            link: {
              type: 'doc',
              id: 'fb/howtos/implementing-a-server/index',
            },
            items: [
              {
                type: 'category',
                label: 'C++',
                items: [
                  'fb/howtos/implementing-a-server/cpp/getting-started',
                  'fb/howtos/implementing-a-server/cpp/implementing-rpcs',
                  'fb/howtos/implementing-a-server/cpp/tailer-services',
                  {
                    type: 'category',
                    label: 'Archived',
                    items: [
                      'fb/howtos/implementing-a-server/cpp/archived/thrift-server-example',
                    ],
                  },
                ],
              },
              'fb/howtos/implementing-a-server/java',
              'fb/howtos/implementing-a-server/python',
            ],
          },
        ],
      },
    ]),
    {
      type: 'category',
      label: 'Interface Definition Language',
      link: {
        type: 'doc',
        id: 'idl/index',
      },
      items: [
        'idl/field-qualifiers',
        'idl/annotations',
        'idl/mixins',
      ],
    },
    {
      type: 'category',
      label: 'Features',
      link: {
        type: 'doc',
        id: 'features/index',
      },
      items: [
        // Features are added to the features/ folder as a flat list so that
        // they can be moved in and out of beta/experimental state without
        // affecting their URLs.

        // Released features:
        {
          type: 'category',
          label: 'Serialization',
          link: {
            type: 'doc',
            id: 'features/serialization/index',
          },
          items: ['features/serialization/protocols'],
        },
        'features/operators',
        'features/universal-name',
        {
          type: 'category',
          label: 'Streaming',
          link: {
            type: 'doc',
            id: 'fb/features/streaming/index',
          },
          items: [
            'fb/features/streaming/sink',
            'fb/features/streaming/java',
            'fb/features/streaming/multicasting',
          ],
        },
        'fb/features/interactions',
        'features/adapters',
        'features/exception',
        'features/compatibility',
        'features/tolerance',

        // Beta features:
        {
          Beta: ['features/any', 'features/field-mask', 'features/terse-write'],
        },
        // Experimental features:
        {
          Experimental: [
            {
              type: 'category',
              label: 'Patch',
              link: {
                type: 'doc',
                id: 'features/patch/patch',
              },
              items: [...fbInternalOnly(['fb/features/patch-compat'])],
            },
            'features/schema',
            ...fbInternalOnly(['fb/features/metadata']),
          ],
        },
      ],
    },
    {
      type: 'category',
      label: 'Languages',
      link: {
        type: 'doc',
        id: 'languages/index',
      },
      collapsible: true,
      collapsed: true,
      items: [
        {
          type: 'category',
          label: 'C++',
          link: {
            type: 'doc',
            id: 'languages/cpp/index',
          },
          items: [
            ...fbInternalOnly([
              'fb/languages/cpp/generated-code',
              'fb/languages/cpp/field-access',
              'fb/languages/cpp/isset-bitpacking',
              'fb/languages/cpp/reflection',
              {
                type: 'category',
                label: 'CodeFrameworks',
                link: {
                  type: 'doc',
                  id: 'fb/languages/cpp/code-frameworks/index',
                },
                items: [
                  'fb/languages/cpp/code-frameworks/binary-contracts',
                  'fb/languages/cpp/code-frameworks/migration',
                ],
              },
              'languages/cpp/channel',
              'languages/cpp/cpp2',
              'languages/cpp/fd-passing',
              'fb/languages/cpp/serialization',
              'fb/languages/cpp/visitation',
              'fb/languages/cpp/hash',
              'fb/languages/cpp/lazy',
              'fb/languages/cpp/protocol-object',
              {
                type: 'category',
                label: 'Frozen2',
                link: {
                  type: 'doc',
                  id: 'fb/languages/cpp/frozen2/index',
                },
                items: ['fb/languages/cpp/frozen2/frozen'],
              },
            ]),
          ],
        },
        ...fbInternalOnly([
          {
            type: 'category',
            label: 'Hack',
            link: {
              type: 'doc',
              id: 'fb/languages/hack/index',
            },
            items: [
              'fb/languages/hack/adding-updating',
              'fb/languages/hack/adding-new-files',
              'fb/languages/hack/auto-sync-for-existing-thrift-files',
              'fb/languages/hack/adding-graphql-and-jsenum-support',
              {
                type: 'category',
                label: 'Using Thrift Clients',
                link: {
                  type: 'doc',
                  id: 'fb/languages/hack/clients/index',
                },
                items: [
                  'fb/languages/hack/clients/streaming',
                  'fb/languages/hack/clients/sink',
                ],
              },
              {
                type: 'category',
                label: 'WWW Thrift Services',
                link: {
                  type: 'doc',
                  id: 'fb/languages/hack/servers/index',
                },
                items: ['fb/languages/hack/servers/sr-setup'],
              },
              'fb/languages/hack/example-of-thrift-clients-in-www',
              'fb/languages/hack/testing-serialization-changes',
            ],
          },
          {
            type: 'category',
            label: 'Java',
            link: {
              type: 'doc',
              id: 'fb/languages/java/index',
            },
            items: [
              'fb/languages/java/caveats',
              'fb/languages/java/client',
              'fb/languages/java/exec',
              'fb/languages/java/generatedfiles',
              'fb/languages/java/hyperthrift',
              'fb/languages/java/interface',
              'fb/languages/java/netty4_background',
              'fb/languages/java/netty4',
              'fb/languages/java/serde',
              'fb/languages/java/legacy_api',
            ],
          },
          {
            type: 'category',
            label: 'Python',
            link: {
              type: 'doc',
              id: 'fb/languages/python/index',
            },
            items: [
              'fb/languages/python/code-frameworks',
              'fb/languages/python/python-capi',
            ],
          },
        ]),
      ],
    },
    ...fbInternalOnly([
      {
        type: 'category',
        label: 'Compiler',
        link: {
          type: 'doc',
          id: 'fb/compiler/index',
        },
        items: ['fb/compiler/buck'],
      },
      {
        type: 'category',
        label: 'Thrift Xrepo Sync',
        link: {
          type: 'doc',
          id: 'fb/developer-tools/xrepo-sync/index',
        },
        items: [
          'fb/developer-tools/xrepo-sync/glossary',
          'fb/developer-tools/xrepo-sync/auto-sync',
          'fb/developer-tools/xrepo-sync/manual-sync',
        ],
      },
      {
        type: 'category',
        label: 'Server',
        link: {
          type: 'doc',
          id: 'fb/server/index',
        },
        items: [
          'fb/server/howto',
          'fb/server/server-lifecycle',
          'fb/server/background-tasks',
          'fb/server/flavors-of-main',
          'fb/server/components',
          'fb/server/transport',
          {
            type: 'category',
            label: 'Interface Protocol',
            link: {
              type: 'doc',
              id: 'fb/server/interface/index',
            },
            items: ['fb/server/interface/rocket'],
          },
          'fb/server/channels',
          'fb/server/threading-models',
          'fb/server/thrift-server-features',
          'fb/server/async-handling',
          'fb/server/configuration',
          {
            type: 'category',
            label: 'Workload prioritization and isolation',
            link: {
              type: 'doc',
              id: 'fb/server/prio-and-isolation/index',
            },
            items: [
              'fb/server/prio-and-isolation/isolation-via-the-sfq-thread-manager',
              'fb/server/prio-and-isolation/priorities-reference',
            ],
          },
          {
            type: 'category',
            label: 'Overload Protection',
            link: {
              type: 'doc',
              id: 'fb/server/overload-protection/index',
            },
            items: [
              {
                type: 'category',
                label: 'CPUConcurrencyController',
                link: {
                  type: 'doc',
                  id: 'fb/server/overload-protection/cpuconcurrencycontroller/index',
                },
                items: [
                  'fb/server/overload-protection/cpuconcurrencycontroller/rollout',
                ],
              },
              {
                type: 'category',
                label: 'DLS',
                link: {
                  type: 'doc',
                  id: 'fb/server/overload-protection/dls/index',
                },
                items: [
                ],
              },
              // 'fb/server/overload-protection/dls/index',
              'fb/server/overload-protection/adaptive-concurrency',
              'fb/server/overload-protection/queue-timeout',
            ],
          },
          {
            type: 'category',
            label: 'Resource Pools',
            link: {
              type: 'doc',
              id: 'fb/server/resource-pools/index',
            },
            items: ['fb/server/resource-pools/api'],
          },
        ],
      },
      {
        type: 'category',
        label: 'Testing',
        link: {
          type: 'doc',
          id: 'fb/testing/index',
        },
        items: [
          'fb/testing/loadgen',
          'fb/testing/stress-test-framework',
          {
            type: 'category',
            label: 'Transport Benchmarks',
            link: {
              type: 'doc',
              id: 'fb/testing/transport-benchmarks/index',
            },
            items: ['fb/testing/transport-benchmarks/adhoc-tests'],
          },
        ],
      },
      {
        type: 'category',
        label: 'Best Practices',
        link: {
          type: 'doc',
          id: 'fb/best-practices/index',
        },
        items: [
          'best-practices/style-guide',
          'fb/best-practices/code-modernization',
          {
            type: 'category',
            label: "You don't need FacebookBase2!",
            link: {
              type: 'doc',
              id: 'fb/best-practices/you-dont-need-facebookbase2/index',
            },
            items: [
              'fb/best-practices/you-dont-need-facebookbase2/facebookbase2-deprecation-bootcamp-instructions',
              'fb/best-practices/you-dont-need-facebookbase2/facebookbase2-deprecation-migration-bootcamp-tasks-oncall-runbook',
              'fb/best-practices/you-dont-need-facebookbase2/facebookbase2-deprecation-migration',
            ],
          },
        ],
      },
      {
        type: 'category',
        label: 'Troubleshooting',
        link: {
          type: 'doc',
          id: 'fb/troubleshooting/index',
        },
        items: [
          'fb/troubleshooting/faq',
          {
            type: 'category',
            label: 'Exception Handling',
            link: {
              type: 'doc',
              id: 'fb/troubleshooting/exception-handling/index',
            },
            items: [
              'fb/troubleshooting/exception-handling/error-classification',
            ],
          },
          'fb/troubleshooting/debugging-tools',
          'fb/troubleshooting/fuzzer',
          {
            type: 'category',
            label: 'Dogpiles',
            link: {
              type: 'doc',
              id: 'fb/troubleshooting/dogpiles/index',
            },
            items: [
              'fb/troubleshooting/dogpiles/index',
              'fb/troubleshooting/dogpiles/io',
              'fb/troubleshooting/dogpiles/server_overload',
              'fb/troubleshooting/dogpiles/development',
            ],
          },
          'fb/troubleshooting/fb303-counters',
        ],
      },
    ]),

    // Documentation for the Thrift team and contributors.
    {
      type: 'category',
      label: 'Contributions',
      link: {
        type: 'doc',
        id: 'contributions/index',
      },
      items: [
        ...fbInternalOnly([
          {
            type: 'category',
            label: 'Thrift Core',
            link: {
              type: 'doc',
              id: 'fb/contributions/thrift-core/index',
            },
            items: [
              {
                type: 'autogenerated',
                dirName: 'fb/contributions/thrift-core',
              },
            ],
          },
          'fb/contributions/flags',
          'fb/contributions/server-logging',
          'fb/contributions/syncing-annotation-library',
          'fb/contributions/jemalloc-profiling-on-server',
          {
            type: 'category',
            label: 'ContextProp',
            link: {
              type: 'doc',
              id: 'fb/contributions/contextprop/index',
            },
            items: [
              'fb/contributions/contextprop/testing',
              'fb/contributions/contextprop/design-and-implementation',
              'fb/contributions/contextprop/extending-fields',
            ],
          },
          'fb/contributions/thrift-repos',
          'fb/contributions/internal-server-logging',
          'fb/contributions/pcap-logging',
          'fb/contributions/regression-test',
          'fb/contributions/rocket-rollout-configs',
          'fb/contributions/runbook',
          'fb/contributions/test-coverage',
          'fb/contributions/testing-dogpiles-locally',
          {
            type: 'category',
            label: 'Thrift Compiler',
            link: {
              type: 'doc',
              id: 'fb/contributions/compiler/index',
            },
            items: [
              'fb/contributions/compiler/updating-thrift-compiler-in-www',
            ],
          },
          'fb/contributions/linter',
          'fb/contributions/oss',
          'fb/contributions/troubleshooting',
          'fb/contributions/xplat',
        ]),
        {
          type: 'category',
          label: 'Conformance',
          link: {
            type: 'doc',
            id: 'contributions/conformance/index',
          },
          items: [
            ...fbInternalOnly(['fb/conformance/quickstart']),
            {
              type: 'category',
              label: 'Test Suite',
              link: {
                type: 'doc',
                id: 'contributions/conformance/testsuite/index',
              },
              items: [
                'contributions/conformance/testsuite/data',
                'contributions/conformance/testsuite/client-rpc',
                'contributions/conformance/testsuite/server-rpc',
              ],
            },
          ],
        },
        'contributions/universal-name',
        'contributions/terse-write',
        'contributions/adapter',
        'contributions/fd-passing',
        'contributions/patch',
        {
          type: 'category',
          label: 'Documentation',
          link: {
            type: 'doc',
            id: 'contributions/documentation/index',
          },
          items: [
            'contributions/documentation/site-structure',
            'contributions/documentation/local-preview',
          ],
        },
      ],
    },

    'glossary',
  ],
};
