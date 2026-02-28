/**
 * (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
 */

/* eslint-disable */

import React from 'react';
import {OssOnly, FbInternalOnly} from 'docusaurus-plugin-internaldocs-fb/internal';
import MDXContent from '@theme/MDXContent';
import Admonition from '@theme/Admonition';

export default function FbInfo({children}) {
  return (
    <FbInternalOnly>
    <Admonition type="info" title="Info">
      <MDXContent>
        {children}
      </MDXContent>
    </Admonition>
    </FbInternalOnly>
  );
}
