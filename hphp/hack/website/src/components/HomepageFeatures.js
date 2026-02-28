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
    href: 'hack-overview',
    Svg: require('../../static/img/hack.svg').default,
    description: (
      <>
        Install Hack, run your first program, and get introduced to the tools.
        <br /><br />
        Website: <a href="https://hacklang.org/" target="_blank">hacklang.org</a>
      </>
    ),
  },
  {
    title: 'HHVM Documentation',
    href: 'hhvm-overview',
    Svg: require('../../static/img/hhvm.svg').default,
    description: (
      <>
        Install HHVM on a supported platform, and learn about ways to run and configure HHVM.
        <br /><br />
        Website: <a href="https://hhvm.com/" target="_blank">hhvm.com</a>
      </>
    ),
  },
  {
    title: 'Library Reference',
    href: 'hsl/overview',
    Svg: require('../../static/img/hsl.svg').default,
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
        <a href={href}>
          <Svg className={styles.featureSvg} alt={title} />
        </a>
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
