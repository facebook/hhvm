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

const {fbContent, fbInternalOnly} = require('docusaurus-plugin-internaldocs-fb/internal');

module.exports = {
  // Public facing TOC (e.g.: Github) defined here directly
  ghSidebar: [
    'intro',
    {
      type: 'category',
      label: 'Get Started',
      link: {
        type: 'doc',
        id: 'get-started/index',
      },
      collapsible: true,
      collapsed: false,
      items: [
        'get-started/quickstart-guide',
      ]
    },
    {
      type: 'category',
      label: 'Specs',
      link: {
        type: 'doc',
        id: 'spec/index'
      },
      collapsible: true,
      collapsed: true,
      items: [
        "spec/about",
        "spec/idl",
        {
          type: 'category',
          label: 'Definitions',
          link: {
            type: 'doc',
            id: 'spec/definition/index',
          },
          items: [
            "spec/definition/interface",
            "spec/definition/program",
            "spec/definition/data",
            "spec/definition/exception",
            "spec/definition/compatibility",
            "spec/definition/tolerance",
            "spec/definition/value",
            "spec/definition/annotation",
          ]
        },
        {
          type: 'category',
          label: 'Protocols',
          link: {
            type: 'doc',
            id: 'spec/protocol/index',
          },
          items: [
            "spec/protocol/channel",
            "spec/protocol/data",
            {
              type: 'category',
              label: 'Interface Protocol',
              link: {
                type: 'doc',
                id: 'spec/protocol/interface/index'
              },
              items: [
                "spec/protocol/interface/rocket"
              ]
            }
          ]
        },
      ]
    },
    {
      type: 'category',
      label: 'Features',
      link: {
        type: 'doc',
        id: 'features/index',
      },
      collapsible: true,
      collapsed: true,
      items: [
        // Release features
        // Features are added to the features/ folder as a flat list
        // Features can be moved in and out of "beta/experimental" state without affecting its URL

        // Beta features
        {
          Beta: [
          'features/adapter',
          'features/any',
        ]},
        //Experimental features
        {
          Experimental: [
            'features/hash',
            'features/patch',
            'features/schema',
            'features/yaml',
          ]
        }
      ]
    },
    {
      type: 'category',
      label: 'Languages',
      link: {
        type: 'doc',
        id: "languages/index",
      },
      collapsible: true,
      collapsed: true,
      items: [
        {
          type: 'category',
          label: 'C++',
          link: {
            type: 'doc',
            id: "languages/cpp/index",
          },
          items: [
            'languages/cpp/channel',
            'languages/cpp/cpp2',
          ]
        }
      ]
    },
    {
      type: 'category',
      label: 'How To',
      link: {
        type: 'doc',
        id: "howtos/index",
      },
      collapsible: true,
      collapsed: true,
      items: [
        'howtos/add-structured-annotations',
      ]
    },
    {
      type: 'category',
      label: 'Tutorials',
      link: {
        type: 'doc',
        id: "tutorials/index",
      },
      collapsible: true,
      collapsed: true,
      items: [
        'tutorials/docstring-annotation',
      ]
    },
    {
      type: 'category',
      label: 'Troubleshooting',
      link: {
        type: 'doc',
        id: "troubleshoot/index", // SD bug?: But naming the folder "troublshooting" causes sidebar to not read this index file
      },
      collapsible: true,
      collapsed: true,
      items: [
        'troubleshoot/errors',
        'troubleshoot/debugging-issues',
      ]
    },
    'references/index', // Will turn this into a category once we have generated ref docs
    'glossary',
    {
      type: 'category',
      label: 'Releases',
      link: {
        type: 'doc',
        id: "releases/index",
      },
      collapsible: true,
      collapsed: true,
      items: [
        'releases/release-notes-v1',
      ]
    },
    {
      type: 'category',
      label: 'Contributions',
      link: {
        type: 'doc',
        id: "contributions/index",
      },
      collapsible: true,
      collapsed: true,
      items: [
        'contributions/conformance/index',
        {
          type: 'category',
          label: 'Test Suite',
          link: {
            type: 'doc',
            id: "contributions/conformance/testsuite/index",
          },
          items: [
          'contributions/conformance/testsuite/data',
          'contributions/conformance/testsuite/client-rpc',
          'contributions/conformance/testsuite/server-rpc',
        ]},
        {
          type: 'category',
          label: 'Content',
          link: {
            type: 'doc',
            id: "contributions/content/index",
          },
          items: [
          'contributions/content/site-structure',
          'contributions/content/local-preview',
        ]},
      ]
    },
  ],
  // Meta employees will also see this additional nav-bar
  // The sidebar items are defined in custom /fb/sidebars.js file
  ...fbInternalOnly(() => require('./fb/sidebars.js')),
};
