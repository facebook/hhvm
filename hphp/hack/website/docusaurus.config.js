/**
 * (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
 */

/* eslint-disable */

const lightCodeTheme = require('prism-react-renderer/themes/github');
const darkCodeTheme = require('prism-react-renderer/themes/vsDark');

// With JSDoc @type annotations, IDEs can provide config autocompletion
/** @type {import('@docusaurus/types').DocusaurusConfig} */
(module.exports = {
  title: 'Hack & HHVM Documentation',
  tagline: 'Moving fast with high-performance Hack, a programming language for building reliable websites at epic scale',
  url: 'https://internalfb.com',
  baseUrl: '/',
  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'throw',
  trailingSlash: true,
  favicon: 'img/favicon.ico',
  organizationName: 'facebook',
  projectName: 'Hack',
  customFields: {
    fbRepoName: 'fbsource',
    ossRepoPath: 'fbcode/hphp/hack/website',
  },

  presets: [
    [
      'docusaurus-plugin-internaldocs-fb/docusaurus-preset',
      /** @type {import('docusaurus-plugin-internaldocs-fb').PresetOptions} */
      ({
        docs: {
          sidebarPath: require.resolve('./sidebars.js'),
          routeBasePath: '/',
          path: '../manual/',
          editUrl: 'https://www.internalfb.com/code/fbsource/fbcode/hphp/hack/website',
        },
        experimentalXRepoSnippets: {
          baseDir: '.',
        },
        staticDocsProject: 'Hack',
        trackingFile: 'fbcode/staticdocs/WATCHED_FILES',
        blog: false,
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      }),
    ],
  ],

  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      navbar: {
        title: 'Hack & HHVM Documentation',
        logo: {
          alt: 'Hack Logo',
          src: 'img/logo.png',
        },
        items: [
          {
            type: 'doc',
            docId: 'hack-overview',
            position: 'left',
            label: 'Hack',
          },
          {
            type: 'doc',
            docId: 'hhvm-overview',
            position: 'left',
            label: 'HHVM',
          },
          {
            type: 'doc',
            docId: 'hsl/overview',
            position: 'left',
            label: 'HSL',
          },
          {
            type: 'doc',
            docId: 'apis/overview',
            position: 'left',
            label: 'API',
          },
          {
            href: 'https://github.com/hhvm/user-documentation',
            label: 'GitHub',
            position: 'right',
          },
        ],
      },
      footer: {
        style: 'dark',
        links: [
          {
            title: 'Hack',
            items: [
              {
                label: 'Overview',
                to: '/hack-overview',
              },
              {
                label: 'Getting Started',
                to: '/hack/getting-started/quick-start',
              },
              {
                label: 'Tools',
                to: '/hack/getting-started/tools',
              },
              {
                label: 'API Reference',
                to: '/apis/overview',
              },
            ],
          },
          {
            title: 'HHVM',
            items: [
              {
                label: 'Overview',
                to: '/hhvm-overview',
              },
              {
                label: 'Installation',
                to: '/hhvm/installation/introduction',
              },
              {
                label: 'Basic Usage',
                to: '/hhvm/basic-usage/introduction',
              },
              {
                label: 'Configuration',
                to: '/hhvm/configuration/introduction',
              },
            ],
          },
          {
            title: 'Community',
            items: [
              {
                label: 'Facebook Group',
                href: 'https://www.facebook.com/groups/hhvm.general',
              },
              {
                label: 'Hack Twitter',
                href: 'https://twitter.com/hacklang',
              },
              {
                label: 'HHVM Twitter',
                href: 'https://twitter.com/HipHopVM',
              },
              {
                label: 'Slack',
                href: 'https://hhvm.com/slack',
              },
            ],
          },
          {
            title: 'Resources',
            items: [
              {
                label: 'Blog',
                href: 'http://hhvm.com/blog',
              },
              {
                label: 'Hack Website',
                href: 'http://hacklang.org/',
              },
              {
                label: 'HHVM Website',
                href: 'http://hhvm.com/',
              },
              {
                label: 'Facebook Page',
                href: 'https://www.facebook.com/hhvm',
              },
            ],
          },
          {
            title: 'GitHub',
            items: [
              {
                label: 'Hack and HHVM',
                href: 'https://github.com/facebook/hhvm',
              },
              {
                label: 'Source for this Site',
                href: 'https://github.com/hhvm/user-documentation',
              },
            ],
          },
        ],
        copyright: `Copyright &#169; ${new Date().getFullYear()} Meta Platforms, Inc.`,
      },
      prism: {
        theme: lightCodeTheme,
        darkTheme: darkCodeTheme,
        additionalLanguages: ['php'],
      },
    }),
});
