package mktdata

import (
	"bytes"
	"fmt"
	"os"
	"testing"
)

func TestPreallocated(t *testing.T) {
	in := &MDIncrementalRefreshBook32{
		TransactTime: 1,
		NoMDEntries: []MDIncrementalRefreshBook32NoMDEntries{
			{
				MDEntryPx:   PRICENULL{},
				MDEntrySize: 5,
				SecurityID:  1,
			},
			{
				MDEntryPx:   PRICENULL{},
				MDEntrySize: 6,
				SecurityID:  1,
			},
		},
	}
	min := NewSbeGoMarshaller()

	buf := &bytes.Buffer{}
	if err := in.Encode(min, buf, true); err != nil {
		fmt.Println("Encoding Error", err)
		os.Exit(1)
	}

	out := &MDIncrementalRefreshBook32{}
	if err := out.Decode(min, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		fmt.Println("Decoding Error", err)
		os.Exit(1)
	}
	if len(out.NoMDEntries) != len(in.NoMDEntries) {
		fmt.Printf("expected %d entries, got %d\n", len(in.NoMDEntries), len(out.NoMDEntries))
		os.Exit(1)
	}

	// decode a message with fewer entries on top of existing message
	in = &MDIncrementalRefreshBook32{
		TransactTime: 1,
		NoMDEntries: []MDIncrementalRefreshBook32NoMDEntries{
			{
				MDEntryPx:   PRICENULL{},
				MDEntrySize: 5,
				SecurityID:  1,
			},
		},
	}
	buf.Reset()
	if err := in.Encode(min, buf, true); err != nil {
		fmt.Println("Encoding Error", err)
		os.Exit(1)
	}

	if err := out.Decode(min, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		fmt.Println("Decoding Error", err)
		os.Exit(1)
	}
	if len(out.NoMDEntries) != len(in.NoMDEntries) {
		fmt.Printf("expected %d entries, got %d\n", len(in.NoMDEntries), len(out.NoMDEntries))
		os.Exit(1)
	}
}
