/**
 * (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
 */

/* eslint-disable */

import React from 'react';
import clsx from 'clsx';
import styles from './HomepageFeatures.module.css';

const FeatureList = [
  {
    title: 'Hack Documentation',
    href: '/hack-overview',
    Svg: require('../../static/img/undraw_docusaurus_mountain.svg').default,
    description: (
      <>
        Install Hack, run your first program, and get introduced to the tools.
      </>
    ),
  },
  {
    title: 'HHVM Documentation',
    href: '/hhvm-overview',
    Svg: require('../../static/img/undraw_docusaurus_tree.svg').default,
    description: (
      <>
        Install HHVM on a supported platform, and learn about ways to run and configure HHVM.
      </>
    ),
  },
  {
    title: 'Library Reference',
    href: '/hsl/overview',
    Svg: require('../../static/img/undraw_docusaurus_react.svg').default,
    description: (
      <>
        Documentation for all functions, classes, interfaces, and traits in the Hack Standard Library (HSL).
      </>
    ),
  },
];

function Feature({Svg, title, description, href}) {
  return (
    <div className={clsx('col col--4')}>
      <div className="text--center">
        <Svg className={styles.featureSvg} alt={title} />
      </div>
      <div className="text--center padding-horiz--md">
        <h3><a href={href}>{title}</a></h3>
        <p>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures() {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className="row">
          {FeatureList.map((props, idx) => (
            <Feature key={idx} {...props} />
          ))}
        </div>
      </div>
    </section>
  );
}
