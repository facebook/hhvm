/*
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
 */

package thrift

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/tls"
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/pem"
	"math/big"
	"net"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"testing"
	"time"

	"github.com/stretchr/testify/require"
)

func getNumFileDesciptors(t *testing.T) int {
	cmd := exec.Command("lsof", "-p", strconv.Itoa(os.Getpid()))
	output, err := cmd.CombinedOutput()
	require.NoError(t, err)

	lines := strings.Split(string(output), "\n")
	require.Greater(t, len(lines), 1)

	return len(lines) - 1 // -1 to account for header line
}

func generateSelfSignedCerts() (clientConfig *tls.Config, serverConfig *tls.Config, err error) {
	caPrivKey, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		return nil, nil, err
	}

	caTemplate := x509.Certificate{
		SerialNumber: big.NewInt(1),
		Subject: pkix.Name{
			Organization:  []string{"Test CA"},
			Country:       []string{"US"},
			Province:      []string{""},
			Locality:      []string{"San Francisco"},
			StreetAddress: []string{""},
			PostalCode:    []string{""},
		},
		NotBefore:             time.Now(),
		NotAfter:              time.Now().Add(365 * 24 * time.Hour),
		IsCA:                  true,
		ExtKeyUsage:           []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth, x509.ExtKeyUsageServerAuth},
		KeyUsage:              x509.KeyUsageDigitalSignature | x509.KeyUsageCertSign,
		BasicConstraintsValid: true,
	}

	caCertDER, err := x509.CreateCertificate(rand.Reader, &caTemplate, &caTemplate, &caPrivKey.PublicKey, caPrivKey)
	if err != nil {
		return nil, nil, err
	}

	caCert, err := x509.ParseCertificate(caCertDER)
	if err != nil {
		return nil, nil, err
	}

	serverPrivKey, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		return nil, nil, err
	}

	serverTemplate := x509.Certificate{
		SerialNumber: big.NewInt(2),
		Subject: pkix.Name{
			Organization:  []string{"Test Server"},
			Country:       []string{"US"},
			Province:      []string{"CA"},
			Locality:      []string{"Menlo Park"},
			StreetAddress: []string{""},
			PostalCode:    []string{""},
		},
		NotBefore:    time.Now(),
		NotAfter:     time.Now().Add(24 * time.Hour),
		SubjectKeyId: []byte{1, 2, 3, 4, 6},
		ExtKeyUsage:  []x509.ExtKeyUsage{x509.ExtKeyUsageServerAuth},
		KeyUsage:     x509.KeyUsageDigitalSignature,
		IPAddresses:  []net.IP{net.IPv4(127, 0, 0, 1), net.IPv6loopback},
		DNSNames:     []string{"localhost"},
	}

	serverCertDER, err := x509.CreateCertificate(rand.Reader, &serverTemplate, caCert, &serverPrivKey.PublicKey, caPrivKey)
	if err != nil {
		return nil, nil, err
	}

	clientPrivKey, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		return nil, nil, err
	}

	clientTemplate := x509.Certificate{
		SerialNumber: big.NewInt(3),
		Subject: pkix.Name{
			Organization:  []string{"Test Client"},
			Country:       []string{"US"},
			Province:      []string{""},
			Locality:      []string{"Menlo Park"},
			StreetAddress: []string{""},
			PostalCode:    []string{""},
		},
		NotBefore:   time.Now(),
		NotAfter:    time.Now().Add(365 * 24 * time.Hour),
		ExtKeyUsage: []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth},
		KeyUsage:    x509.KeyUsageDigitalSignature,
	}

	clientCertDER, err := x509.CreateCertificate(rand.Reader, &clientTemplate, caCert, &clientPrivKey.PublicKey, caPrivKey)
	if err != nil {
		return nil, nil, err
	}

	serverCertPEM := pem.EncodeToMemory(&pem.Block{
		Type:  "CERTIFICATE",
		Bytes: serverCertDER,
	})

	serverKeyPEM := pem.EncodeToMemory(&pem.Block{
		Type:  "RSA PRIVATE KEY",
		Bytes: x509.MarshalPKCS1PrivateKey(serverPrivKey),
	})

	clientCertPEM := pem.EncodeToMemory(&pem.Block{
		Type:  "CERTIFICATE",
		Bytes: clientCertDER,
	})

	clientKeyPEM := pem.EncodeToMemory(&pem.Block{
		Type:  "RSA PRIVATE KEY",
		Bytes: x509.MarshalPKCS1PrivateKey(clientPrivKey),
	})

	caCertPEM := pem.EncodeToMemory(&pem.Block{
		Type:  "CERTIFICATE",
		Bytes: caCertDER,
	})

	serverCert, err := tls.X509KeyPair(serverCertPEM, serverKeyPEM)
	if err != nil {
		return nil, nil, err
	}

	clientCert, err := tls.X509KeyPair(clientCertPEM, clientKeyPEM)
	if err != nil {
		return nil, nil, err
	}

	caCertPool := x509.NewCertPool()
	caCertPool.AppendCertsFromPEM(caCertPEM)

	serverConfig = &tls.Config{
		Certificates: []tls.Certificate{serverCert},
		ClientAuth:   tls.RequireAndVerifyClientCert,
		ClientCAs:    caCertPool,
	}

	clientConfig = &tls.Config{
		Certificates: []tls.Certificate{clientCert},
		RootCAs:      caCertPool,
		ServerName:   "localhost",
	}

	return clientConfig, serverConfig, nil
}

func getClientTransportOption(serverTransport TransportID) ClientOption {
	switch serverTransport {
	case TransportIDHeader:
		return WithHeader()
	case TransportIDUpgradeToRocket:
		return WithUpgradeToRocket()
	case TransportIDRocket:
		return WithRocket()
	default:
		panic("unsupported transport!")
	}
}
