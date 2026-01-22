/**
 * (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
 */

/* eslint-disable */

const lightCodeTheme = require('prism-react-renderer').themes.github;
const darkCodeTheme = require('prism-react-renderer').themes.vsDark;
const { fbContent } = require('docusaurus-plugin-internaldocs-fb/internal');

/** @type {import('@docusaurus/types').DocusaurusConfig} */
(module.exports = {
  title: 'Hack & HHVM Documentation',
  tagline: 'Moving fast with high-performance Hack, a programming language for building reliable websites at epic scale',
  url: 'https://docs.hhvm.com',
  baseUrl: process.env.DOCUSAURUS_BASE_URL || '/',
  onBrokenLinks: 'throw',
  trailingSlash: true,
  favicon: 'img/favicon.ico',
  organizationName: 'facebook',
  projectName: 'hackstack',
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
          routeBasePath: '/',
          path: '../manual/',
          editUrl: fbContent({
              internal: 'https://www.internalfb.com/code/fbsource/fbcode/hphp/hack/manual/',
              external: 'https://github.com/hhvm/user-documentation/edit/main/',
          }),
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
        enableEditor: true,
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
          height: 60,
        },
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
