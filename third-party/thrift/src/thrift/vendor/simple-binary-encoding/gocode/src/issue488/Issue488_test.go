package issue488

import (
	"testing"
)

func TestPresence(t *testing.T) {
	issue488 := new(Issue488)
	v := new(VarDataEncoding)
	issue488.VarData = make([]uint8, 3)
	issue488.VarData[0] = v.VarDataMinValue()
	issue488.VarData[1] = 42
	issue488.VarData[2] = v.VarDataMaxValue()
	if err := issue488.RangeCheck(0, 0); err != nil {
		t.Log("RangeCheck failed", err)
		t.Fail()
	}
}
