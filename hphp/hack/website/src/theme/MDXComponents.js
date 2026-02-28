import React from 'react';
// Import the original mapper
import MDXComponents from '@theme-original/MDXComponents';
import Col50 from '@site/src/components/Col50';
import EndCol from '@site/src/components/EndCol';
import FbCaution from '@site/src/components/FbCaution';
import FbHistorical from '@site/src/components/FbHistorical';
import FbInfo from '@site/src/components/FbInfo';

export default {
  // Re-use the default mapping
  ...MDXComponents,
  Col50,
  EndCol,
  FbCaution,
  FbHistorical,
  FbInfo,
};
