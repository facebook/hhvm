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
	"math"
	"runtime"
	"sync"
	"testing"
	"time"
)

// lots of emperically generated test data which represents
// second duration
var testData = []float64{
	0.003044, 0.006794, 0.004844, 0.004385, 0.004501, 0.009076,
	0.005057, 0.007750, 0.004652, 0.004681, 0.005325, 0.003920,
	0.005146, 0.003984, 0.007712, 0.011444, 0.000447, 0.000567,
	0.004297, 0.000651, 0.008217, 0.000619, 0.000835, 0.000903,
	0.000448, 0.001122, 0.001341, 0.000565, 0.012153, 0.001747,
	0.000885, 0.007054, 0.009308, 0.001157, 0.000625, 0.012506,
	0.001687, 0.001432, 0.005683, 0.001446, 0.000611, 0.002149,
	0.000992, 0.001756, 0.002207, 0.000420, 0.000671, 0.002668,
	0.002795, 0.006887, 0.000939, 0.001259, 0.002379, 0.001919,
	0.001892, 0.000846, 0.001910, 0.001787, 0.001305, 0.015553,
	0.001585, 0.001516, 0.001011, 0.001760, 0.001848, 0.001065,
	0.002182, 0.003043, 0.002995, 0.001957, 0.001295, 0.001083,
	0.001734, 0.005197, 0.000825, 0.001136, 0.000582, 0.001643,
	0.000397, 0.001460, 0.002611, 0.001513, 0.000645, 0.002374,
	0.001281, 0.000428, 0.001782, 0.000433, 0.003596, 0.001654,
	0.001041, 0.006240, 0.003242, 0.004764, 0.003607, 0.004955,
	0.002805, 0.001672, 0.003827, 0.002896, 0.007621, 0.003342,
	0.001635, 0.004625, 0.001844, 0.001869, 0.003663, 0.005050,
	0.004366, 0.005639, 0.008460, 0.002341, 0.004019, 0.003395,
	0.006324, 0.003358, 0.003089, 0.005804, 0.003438, 0.009606,
	0.003526, 0.006962, 0.003858, 0.004220, 0.006507, 0.006751,
	0.004901, 0.003901, 0.000868, 0.001409, 0.006370, 0.005663,
	0.005570, 0.008183, 0.000398, 0.001450, 0.005227, 0.000604,
	0.000940, 0.006520, 0.001118, 0.007445, 0.013064, 0.006938,
	0.013062, 0.012161, 0.000419, 0.007049, 0.000148, 0.000188,
	0.000635, 0.000673, 0.000834, 0.000866, 0.000897, 0.000896,
	0.000262, 0.001145, 0.001131, 0.001141, 0.000242, 0.001182,
	0.001023, 0.000850, 0.000433, 0.001476, 0.001481, 0.000281,
	0.001711, 0.000243, 0.000860, 0.001840, 0.000657, 0.000813,
	0.002081, 0.001057, 0.001704, 0.001329, 0.000612, 0.000713,
	0.000788, 0.001440, 0.002600, 0.000247, 0.000915, 0.000318,
	0.000539, 0.000790, 0.003046, 0.002203, 0.000861, 0.002029,
	0.002189, 0.000283, 0.000840, 0.001714, 0.000860, 0.001972,
	0.001977, 0.001030, 0.001492, 0.001128, 0.003748, 0.000869,
	0.000381, 0.003711, 0.000776, 0.000608, 0.000376, 0.000364,
	0.000131, 0.000795, 0.001480, 0.000890, 0.001418, 0.000162,
	0.000777, 0.000573, 0.001418, 0.001944, 0.000856, 0.004783,
	0.001067, 0.002895, 0.002394, 0.003806, 0.002586, 0.002632,
	0.004509, 0.002584, 0.003876, 0.004610, 0.003510, 0.005490,
	0.003730, 0.003685, 0.005294, 0.004731, 0.005129, 0.000668,
	0.006290, 0.004986, 0.004033, 0.003738, 0.005052, 0.004752,
	0.004191, 0.004451, 0.004308, 0.005491, 0.006376, 0.006002,
	0.004995, 0.003931, 0.000216, 0.005463, 0.004703, 0.006372,
	0.000383, 0.004020, 0.000421, 0.000824, 0.007811, 0.000885,
	0.005075, 0.000151, 0.000588, 0.001101, 0.002441, 0.001203,
	0.004102, 0.005708, 0.000848, 0.000714, 0.001403, 0.009276,
	0.001664, 0.001018, 0.006536, 0.000443, 0.001124, 0.001672,
	0.002220, 0.001249, 0.001002, 0.000704, 0.000758, 0.001857,
	0.001320, 0.001820, 0.000584, 0.001731, 0.000295, 0.000602,
	0.001708, 0.001176, 0.000420, 0.001771, 0.006431, 0.006962,
	0.001520, 0.009455, 0.001622, 0.002121, 0.003197, 0.001854,
	0.001231, 0.002181, 0.002071, 0.001384, 0.004487, 0.000549,
	0.001760, 0.008779, 0.002023, 0.000840, 0.002217, 0.000416,
	0.001210, 0.001035, 0.000381, 0.004519, 0.000292, 0.000927,
	0.000963, 0.001122, 0.003082, 0.001024, 0.002628, 0.000635,
	0.000721, 0.001269, 0.002223, 0.002138, 0.000184, 0.000873,
	0.000362, 0.001320, 0.003684, 0.006103, 0.002340, 0.001869,
	0.004096, 0.003521, 0.004218, 0.001257, 0.001362, 0.004372,
	0.002731, 0.002867, 0.002712, 0.002629, 0.008302, 0.002854,
	0.001988, 0.006196, 0.002012, 0.003196, 0.007766, 0.005120,
	0.003257, 0.003540, 0.003440, 0.004230, 0.004640, 0.002691,
	0.003793, 0.006121, 0.004665, 0.004063, 0.003048, 0.005212,
	0.008305, 0.004145, 0.000861, 0.004304, 0.005532, 0.007282,
	0.003821, 0.004795, 0.006082, 0.007526, 0.009091, 0.020159,
	0.000280, 0.005130, 0.000640, 0.008618, 0.000822, 0.001282,
	0.009481, 0.001127, 0.001270, 0.000502, 0.001003, 0.007321,
	0.001548, 0.001582, 0.001593, 0.001603, 0.001590, 0.001008,
	0.000242, 0.000418, 0.000583, 0.001091, 0.001374, 0.001733,
	0.002099, 0.001141, 0.002233, 0.001135, 0.002038, 0.000827,
	0.000433, 0.000768, 0.000785, 0.000504, 0.000441, 0.000918,
	0.001740, 0.001030, 0.008779, 0.008800, 0.002918, 0.002852,
	0.001328, 0.001985, 0.001795, 0.002961, 0.003496, 0.012049,
}

func TestTimeseriesMisconfig(t *testing.T) {
	ts := NewTimingSeries(nil)
	// you can't summarize a larger period than you are recording
	_, err := ts.Summarize(DefaultConfig.History + DefaultConfig.Interval)
	if err == nil {
		t.Fatalf("expected error")
	}
	// you can summarize all of your history
	_, err = ts.Summarize(DefaultConfig.History)
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestBasicTimeseries(t *testing.T) {
	// first let's just verify that the summary data matches
	// via the timing abstraction and external analysis.
	config := TimingConfig{
		History:   10 * time.Second,
		Precision: time.Microsecond,
		// To excercise the mechanisms in play a bit, we set the
		// interval size very to small to cause data to
		// be spread across multiple bucktes.
		Interval: time.Microsecond * time.Duration(10),
	}
	ts := NewTimingSeries(&config)
	var sum float64
	for _, datum := range testData {
		sum += datum
		ts.Record(time.Duration(datum * float64(time.Second)))
	}
	avg := sum / float64(len(testData))
	// now wait at least one interval so summary will include all data
	time.Sleep(config.Interval)
	summary, err := ts.Summarize(10 * time.Second)
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if want, got := int(len(testData)), int(summary.Count); want != got {
		t.Fatalf("want %v got %v", want, got)
	}
	// verify error is < 10x precision
	if want, got := avg, summary.Average.Seconds(); !inEpsilon(want, got, config.Precision.Seconds()*10.) {
		t.Fatalf("verify error is < 10x precision: want %v got %v", want, got)
	}
}

func TestMultiThreadedTimeseries(t *testing.T) {
	// first let's just verify that the summary data matches
	// via the timing abstraction and external analysis.
	config := TimingConfig{
		History:   10 * time.Second,
		Precision: time.Microsecond,
		// To excercise the mechanisms in play a bit, we set the
		// interval size very to small to cause data to
		// be spread across multiple bucktes.
		Interval: time.Millisecond,
	}
	ts := NewTimingSeries(&config)

	var wait sync.WaitGroup

	dataChan := make(chan float64, 1000)

	// allocate consumers
	for i := 0; i < runtime.NumCPU(); i++ {
		wait.Add(1)
		go func() {
			for datum := range dataChan {
				ts.Record(time.Duration(datum * float64(time.Second)))
			}
			wait.Done()
		}()
	}

	// now inflate data values a bit to stress the system
	repeatDataN := runtime.NumCPU() * 10
	var sum, max, min float64
	for i := 0; i < repeatDataN; i++ {
		for j, datum := range testData {
			sum += datum
			if (i == 0 && j == 0) || min > datum {
				min = datum
			}
			if max < datum {
				max = datum
			}
			dataChan <- datum
		}
	}
	close(dataChan)
	wait.Wait()
	// now wait a couple intervals so summary will include all data
	time.Sleep(config.Interval * 2.)

	summary, err := ts.Summarize(config.History)
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	avg := sum / float64(len(testData)*repeatDataN)

	if want, got := int(len(testData)*repeatDataN), int(summary.Count); want != got {
		t.Fatalf("verify the correct number of data points: want %v got %v", want, got)
	}
	if want, got := avg, summary.Average.Seconds(); !inEpsilon(want, got, config.Precision.Seconds()*10.) {
		t.Fatalf("verify avg error is < 10x precision: want %v got %v", want, got)
	}
	if want, got := min, summary.Minimum.Seconds(); !inEpsilon(want, got, config.Precision.Seconds()) {
		t.Fatalf("verify min within precision: want %v got %v", want, got)
	}
	if want, got := max, summary.Maximum.Seconds(); !inEpsilon(want, got, config.Precision.Seconds()) {
		t.Fatalf("verify max within precision: want %v got %v", want, got)
	}
	if want, got := config.History, summary.Period; want != got {
		t.Fatalf("verify period correctly reported: want %v got %v", want, got)
	}
	if want, got := uint64(0), summary.Success; want != got {
		t.Fatalf("verify success is zero when not in use: want %v got %v", want, got)
	}
	if want, got := uint64(0), summary.Fail; want != got {
		t.Fatalf("verify fail is zero when not in use: want %v got %v", want, got)
	}
}

func inEpsilon(a, b, epsilon float64) bool {
	if a == 0 {
		return false
	}
	actual := math.Abs(a-b) / math.Abs(a)
	return actual < epsilon
}
