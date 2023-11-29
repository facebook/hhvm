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

// @ts-check
// Note: type annotations allow type checking and IDEs autocompletion
const {
  fbContent,
  fbInternalOnly
} = require('docusaurus-plugin-internaldocs-fb/internal');
/** @type {import('@docusaurus/types').Config} */

const config = {
  title: 'Facebook Thrift',
  tagline: 'A serialization and RPC framework',
  url: 'https://www.internalfb.com',
  baseUrl: '/',
  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'warn',
  favicon: 'img/favicon.ico',
  // GitHub pages deployment config.
  // If you aren't using GitHub pages, you don't need these.
  organizationName: 'facebook',
  projectName: 'fbthrift',
  presets: [[require.resolve('docusaurus-plugin-internaldocs-fb/docusaurus-preset'), {
    docs: {
      path: '../doc',
      sidebarPath: require.resolve('./sidebars.js'),
      editUrl: fbContent({
        internal: 'https://www.internalfb.com/intern/diffusion/FBS/browse/master/fbcode/thrift/website/',
        external: 'https://github.com/facebook/fbthrift/blob/main/thrift/website'
      })
    },
    staticDocsProject: 'thrift',
    theme: {
      customCss: [require.resolve('./src/css/custom.css')],
    },
    trackingFile: 'fbcode/staticdocs/WATCHED_FILES',
    'remark-code-snippets': {
      baseDir: '..'
    },
    enableEditor: true
  }]],
  customFields: {
    fbRepoName: 'fbsource',
    ossRepoPath: 'fbcode/thrift'
  },
  themeConfig:
  /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
  {
    navbar: {
      title: 'Facebook Thrift',
      logo: {
        alt: 'Facebook Thrift Logo',
        src: 'img/logo.svg'
      },
      items: [
        {
          type: 'doc',
          docId: 'intro',
          position: 'left',
          label: 'Documentation'
        },
        {
          position: 'left',
          label: 'API',
          items: [ // This generates a dropdown menu for API
            {
              to: 'cpp_api_toc',
              label: 'C++',
            }
          ]
        },
        // Please keep GitHub link to the right for consistency.
        {
          href: 'https://github.com/facebook/fbthrift',
          label: 'GitHub',
          position: 'right'
        }
      ]
    },
    footer: {
      style: 'dark',
      links: [{
        title: 'Learn',
        items: [{
          label: 'Docs',
          to: 'docs/'
        }]
      }, {
        title: 'Community',
        items: [{
          label: 'Stack Overflow',
          href: 'https://stackoverflow.com/questions/tagged/thrift'
        }]
      }, {
        title: 'More',
        items: [{
          label: 'Blog',
          href: 'https://engineering.fb.com/2014/02/20/open-source/under-the-hood-building-and-open-sourcing-fbthrift/'
        }, {
          label: 'GitHub',
          href: 'https://github.com/facebook/fbthrift'
        }]
      }, {
        title: 'Legal',
        // Please do not remove the privacy and terms, it's a legal requirement.
        items: [{
          label: 'Privacy',
          href: 'https://opensource.facebook.com/legal/privacy/'
        }, {
          label: 'Terms',
          href: 'https://opensource.facebook.com/legal/terms/'
        }, {
          label: 'Data Policy',
          href: 'https://opensource.facebook.com/legal/data-policy/'
        }, {
          label: 'Cookie Policy',
          href: 'https://opensource.facebook.com/legal/cookie-policy/'
        }]
      }],
      logo: {
        alt: 'Facebook Open Source Logo',
        src: 'img/oss_logo.png',
        href: 'https://opensource.facebook.com'
      },
      // Please do not remove the credits, help to publicize Docusaurus :)
      copyright: `Copyright © ${new Date().getFullYear()} Meta Platforms, Inc. Built with Docusaurus.`
    },
    prism: {
      theme: require('prism-react-renderer/themes/github'),
      additionalLanguages: ['java', 'php'],
    },
  }
};
module.exports = config;
