/**
 * (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
 */

/* eslint-disable */

const lightCodeTheme = require('prism-react-renderer').themes.github;
const darkCodeTheme = require('prism-react-renderer').themes.vsDark;

// With JSDoc @type annotations, IDEs can provide config autocompletion
/** @type {import('@docusaurus/types').DocusaurusConfig} */
(module.exports = {
  title: 'Hack & HHVM Documentation',
  tagline: 'Moving fast with high-performance Hack, a programming language for building reliable websites at epic scale',
  url: 'https://internalfb.com',
  baseUrl: process.env.DOCUSAURUS_BASE_URL || '/',
  onBrokenLinks: 'warn', // TODO: change back to 'throw' ones apis/* (and hsl/*) have been populated
  trailingSlash: true,
  favicon: 'img/favicon.ico',
  organizationName: 'facebook',
  projectName: 'hack',
  customFields: {
    fbRepoName: 'fbsource',
    ossRepoPath: 'fbcode/hphp/hack/website',
  },
  markdown: {
   hooks: {
      onBrokenMarkdownLinks: 'warn',
      onBrokenMarkdownImages: 'throw',
    },
    anchors: {
      maintainCase: false,
    },
  },
  presets: [
    [
      'docusaurus-plugin-internaldocs-fb/docusaurus-preset',
      /** @type {import('docusaurus-plugin-internaldocs-fb').PresetOptions} */
      ({
        docs: {
          sidebarPath: require.resolve('./sidebars.js'),
          routeBasePath: 'docs',
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
        logo: {
          alt: 'Meta Open Source Logo',
          src: 'img/meta_open_source_logo.svg',
          target: '_blank',
          href: 'https://opensource.fb.com/',
        },
        links: [
          {
            title: 'Hack',
            items: [
              {
                label: 'Overview',
                to: '/docs/hack-overview',
              },
              {
                label: 'Getting Started',
                to: '/docs/hack/getting-started/quick-start',
              },
              {
                label: 'Tools',
                to: '/docs/hack/getting-started/tools',
              },
              {
                label: 'API Reference',
                to: '/docs/apis/overview',
              },
            ],
          },
          {
            title: 'HHVM',
            items: [
              {
                label: 'Overview',
                to: '/docs/hhvm-overview',
              },
              {
                label: 'Installation',
                to: '/docs/hhvm/installation/introduction',
              },
              {
                label: 'Basic Usage',
                to: '/docs/hhvm/basic-usage/introduction',
              },
              {
                label: 'Configuration',
                to: '/docs/hhvm/configuration/introduction',
              },
            ],
          },
          {
            title: 'Community',
            items: [
              {
                label: 'Hack & HHVM GitHub',
                href: 'https://github.com/facebook/hhvm',
              },
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
            ],
          },
          {
            title: 'Resources',
            items: [
              {
                label: 'Source for this Site',
                href: 'https://github.com/hhvm/user-documentation',
              },
              {
                label: 'HHVM Blog',
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
            ],
          },
          {
            title: 'Legal',
            items: [
              {
              label: 'Privacy',
                href: 'https://opensource.facebook.com/legal/privacy/',
                target: '_blank',
                rel: 'noreferrer noopener',
              },
              {
                label: 'Terms',
                href: 'https://opensource.facebook.com/legal/terms/',
                target: '_blank',
                rel: 'noreferrer noopener',
              },
            ],
          },
        ],
        copyright: `Copyright &#169; ${new Date().getFullYear()} Meta Platforms, Inc. Built with Docusaurus.`,
      },
      prism: {
        theme: lightCodeTheme,
        darkTheme: darkCodeTheme,
        additionalLanguages: ['php', 'bash', 'markdown', 'json', 'sql', 'toml', 'yaml'],
      },
    }),
});
