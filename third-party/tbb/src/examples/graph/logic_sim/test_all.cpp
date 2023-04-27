/*
    Copyright (c) 2005-2018 Intel Corporation

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.




*/

#include "tbb/tbb_config.h"
#include <cstdio>

#include "../../common/utility/utility.h"

#if __TBB_FLOW_GRAPH_CPP11_FEATURES

#if _MSC_VER
#pragma warning (disable: 4503) // Suppress "decorated name length exceeded, name was truncated" warning
#endif

#define USE_TWO_BIT_FULL_ADDER 1

#include "basics.h"
#include "one_bit_adder.h"
#if USE_TWO_BIT_FULL_ADDER
#include "two_bit_adder.h"
#else
#include "four_bit_adder.h"
#endif
#include "D_latch.h"
#include <cassert>

// User-specified globals with default values
bool verbose = false;            // prints bin details and other diagnostics to screen
bool silent = false;             // suppress all output except for time

int get_default_num_threads() {
    static int threads = 0;
    if (threads == 0)
        threads = tbb::task_scheduler_init::default_num_threads();
    return threads;
}

#endif // __TBB_FLOW_GRAPH_CPP11_FEATURES

int main(int argc, char *argv[]) {
#if __TBB_FLOW_GRAPH_CPP11_FEATURES
    try {
        utility::thread_number_range threads(get_default_num_threads);
        utility::parse_cli_arguments(argc, argv,
                                     utility::cli_argument_pack()
                                     //"-h" option for displaying help is present implicitly
                                     .positional_arg(threads,"#threads",utility::thread_number_range_desc)
                                     .arg(verbose,"verbose","   print diagnostic output to screen")
                                     .arg(silent,"silent","    limits output to timing info; overrides verbose")
        );

        if (silent) verbose = false;  // make silent override verbose

        tick_count start = tick_count::now();
        for(int p = threads.first; p <= threads.last; p = threads.step(p)) {
            task_scheduler_init init(p);
            if (!silent)  cout << "graph test running on " << p << " threads.\n";
            
            graph g;

            { // test buffer: 0, 1
                buffer b(g);
                toggle input(g);
                led output(g, "OUTPUT", false); // false means we will explicitly call display to see LED
                
                make_edge(input.get_out(), input_port<0>(b));
                make_edge(output_port<0>(b), output.get_in());
                
                if (!silent) printf("Testing buffer...\n");
                input.activate(); // 0
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == low);
                input.flip(); // 1
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == high);
            }

            { // test not_gate: 0, 1
                not_gate n(g);
                toggle input(g);
                led output(g, "OUTPUT", false);
                
                make_edge(input.get_out(), input_port<0>(n));
                make_edge(output_port<0>(n), output.get_in());
                
                if (!silent) printf("Testing not_gate...\n");
                input.activate(); // 0
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == high);
                input.flip(); // 1
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == low);
            }

            { // test two-input and_gate: 00, 01, 10, 11
                and_gate<2> a(g);
                toggle input0(g);
                toggle input1(g);
                led output(g, "OUTPUT", false);
                
                make_edge(input0.get_out(), input_port<0>(a));
                make_edge(input1.get_out(), input_port<1>(a));
                make_edge(output_port<0>(a), output.get_in());
                
                if (!silent) printf("Testing and_gate...\n");
                input1.activate();  input0.activate();  // 0 0
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == low);
                input0.flip();  // 0 1
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == low);
                input1.flip(); input0.flip();  // 1 0
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == low);
                input0.flip();  // 1 1
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == high);
            }

            { // test three-input or_gate: 000, 001, 010, 100, 011, 101, 110, 111
                or_gate<3> o(g);
                toggle input0(g);
                toggle input1(g);
                toggle input2(g);
                led output(g, "OUTPUT", false);
                
                make_edge(input0.get_out(), input_port<0>(o));
                make_edge(input1.get_out(), input_port<1>(o));
                make_edge(input2.get_out(), input_port<2>(o));
                make_edge(output_port<0>(o), output.get_in());
                
                if (!silent) printf("Testing or_gate...\n");
                input2.activate();  input1.activate();  input0.activate();  // 0 0 0
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == low);
                input0.flip();  // 0 0 1
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == high);
                input1.flip(); input0.flip();  // 0 1 0
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == high);
                input2.flip();  input1.flip();  // 1 0 0
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == high);
                input2.flip();  input1.flip();  input0.flip();  // 0 1 1
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == high);
                input2.flip();  input1.flip();  // 1 0 1
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == high);
                input1.flip();  input0.flip();  // 1 1 0
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == high);
                input0.flip();  // 1 1 1
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == high);
            }

            { // test two-input xor_gate: 00, 01, 10, 11
                xor_gate<2> x(g);
                toggle input0(g);
                toggle input1(g);
                led output(g, "OUTPUT", false);
                
                make_edge(input0.get_out(), input_port<0>(x));
                make_edge(input1.get_out(), input_port<1>(x));
                make_edge(output_port<0>(x), output.get_in());
                
                if (!silent) printf("Testing xor_gate...\n");
                input1.activate();  input0.activate();  // 0 0
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == low);
                input0.flip();  // 0 1
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == high);
                input1.flip();  input0.flip();  // 1 0
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == high);
                input0.flip();  // 1 1
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == low);
            }


            { // test two-input nor_gate: 00, 01, 10, 11
                nor_gate<2> n(g);
                toggle input0(g);
                toggle input1(g);
                led output(g, "OUTPUT", false);
                
                make_edge(input0.get_out(), input_port<0>(n));
                make_edge(input1.get_out(), input_port<1>(n));
                make_edge(output_port<0>(n), output.get_in());
                
                if (!silent) printf("Testing nor_gate...\n");
                input1.activate();  input0.activate();  // 0 0
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == high);
                input0.flip();  // 0 1
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == low);
                input1.flip();  input0.flip();  // 1 0
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == low);
                input0.flip();  // 1 1
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == low);
            }

            { // test steady_signal and digit
                steady_signal input0(g, high);
                steady_signal input1(g, low);
                and_gate<2> a(g);
                or_gate<2> o(g);
                xor_gate<2> x(g);
                nor_gate<2> n(g);
                digit output(g, "OUTPUT", false);
                
                make_edge(input0.get_out(), input_port<0>(a));
                make_edge(input1.get_out(), input_port<1>(a));
                make_edge(output_port<0>(a), input_port<0>(output));

                make_edge(input0.get_out(), input_port<0>(o));
                make_edge(input1.get_out(), input_port<1>(o));
                make_edge(output_port<0>(o), input_port<1>(output));

                make_edge(input0.get_out(), input_port<0>(x));
                make_edge(input1.get_out(), input_port<1>(x));
                make_edge(output_port<0>(x), input_port<2>(output));

                make_edge(input0.get_out(), input_port<0>(n));
                make_edge(input1.get_out(), input_port<1>(n));
                make_edge(output_port<0>(n), input_port<3>(output));
                
                if (!silent) printf("Testing steady_signal...\n");
                input0.activate();  // 1
                input1.activate();  // 0
                g.wait_for_all();
                if (!silent) output.display();
                assert(output.get_value() == 6);
            }

            { // test push_button
                push_button p(g);
                buffer b(g);
                led output(g, "OUTPUT", !silent); // true means print all LED state changes

                make_edge(p.get_out(), input_port<0>(b));
                make_edge(output_port<0>(b), output.get_in());

                if (!silent) printf("Testing push_button...\n");
                p.press();
                p.release();
                p.press();
                p.release();
                g.wait_for_all();
            }

            { // test one_bit_adder
                one_bit_adder my_adder(g);
                toggle A(g);
                toggle B(g);
                toggle CarryIN(g);
                led Sum(g, "SUM");
                led CarryOUT(g, "CarryOUT");
                
                make_edge(A.get_out(), input_port<P::A0>(my_adder));
                make_edge(B.get_out(), input_port<P::B0>(my_adder));
                make_edge(CarryIN.get_out(), input_port<P::CI>(my_adder));
                make_edge(output_port<P::S0>(my_adder), Sum.get_in());
                make_edge(output_port<1>(my_adder), CarryOUT.get_in());
                
                A.activate();
                B.activate();
                CarryIN.activate();
                
                if (!silent) printf("A on\n");
                A.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == high) && (CarryOUT.get_value() == low));
                
                if (!silent) printf("A off\n");
                A.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == low) && (CarryOUT.get_value() == low));
                
                if (!silent) printf("B on\n");
                B.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == high) && (CarryOUT.get_value() == low));
                if (!silent) printf("B off\n");
                B.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == low) && (CarryOUT.get_value() == low));
                
                if (!silent) printf("CarryIN on\n");
                CarryIN.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == high) && (CarryOUT.get_value() == low));
                if (!silent) printf("CarryIN off\n");
                CarryIN.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == low) && (CarryOUT.get_value() == low));
                
                if (!silent) printf("A&B on\n");
                A.flip();
                B.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == low) && (CarryOUT.get_value() == high));
                if (!silent) printf("A&B off\n");
                A.flip();
                B.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == low) && (CarryOUT.get_value() == low));
                
                if (!silent) printf("A&CarryIN on\n");
                A.flip();
                CarryIN.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == low) && (CarryOUT.get_value() == high));
                if (!silent) printf("A&CarryIN off\n");
                A.flip();
                CarryIN.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == low) && (CarryOUT.get_value() == low));
                
                if (!silent) printf("B&CarryIN on\n");
                B.flip();
                CarryIN.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == low) && (CarryOUT.get_value() == high));
                if (!silent) printf("B&CarryIN off\n");
                B.flip();
                CarryIN.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == low) && (CarryOUT.get_value() == low));
                
                if (!silent) printf("A&B&CarryIN on\n");
                A.flip();
                B.flip();
                CarryIN.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == high) && (CarryOUT.get_value() == high));
                if (!silent) printf("A&B&CarryIN off\n");
                A.flip();
                B.flip();
                CarryIN.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == low) && (CarryOUT.get_value() == low));
            }

#if USE_TWO_BIT_FULL_ADDER
            { // test two_bit_adder
                if (!silent) printf("testing two_bit adder\n");
                two_bit_adder two_adder(g);
                std::vector<toggle> A(2, toggle(g));
                std::vector<toggle> B(2, toggle(g));
                toggle CarryIN(g);
                digit Sum(g, "SUM");
                led CarryOUT(g, "CarryOUT");

                make_edge(A[0].get_out(), input_port<P::A0>(two_adder));
                make_edge(B[0].get_out(), input_port<P::B0>(two_adder));
                make_edge(output_port<P::S0>(two_adder), input_port<0>(Sum));

                make_edge(A[1].get_out(), input_port<P::A1>(two_adder));
                make_edge(B[1].get_out(), input_port<P::B1>(two_adder)); 
                make_edge(output_port<P::S1>(two_adder), input_port<1>(Sum));

                make_edge(CarryIN.get_out(), input_port<P::CI>(two_adder));
                make_edge(output_port<P::CO>(two_adder), CarryOUT.get_in());

                // Activate all switches at low state
                for (int i=0; i<2; ++i) {
                    A[i].activate();
                    B[i].activate();
                }
                CarryIN.activate();

                if (!silent) printf("1+0\n");
                A[0].flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == 1) && (CarryOUT.get_value() == low));

                if (!silent) printf("0+1\n");
                A[0].flip();
                B[0].flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == 1) && (CarryOUT.get_value() == low));
            }
#else
            { // test four_bit_adder
                four_bit_adder four_adder(g);
                std::vector<toggle> A(4, toggle(g));
                std::vector<toggle> B(4, toggle(g));
                toggle CarryIN(g);
                digit Sum(g, "SUM");
                led CarryOUT(g, "CarryOUT");

                    make_edge(A[0].get_out(), input_port<P::A0>(four_adder));
                    make_edge(B[0].get_out(), input_port<P::B0>(four_adder));
                    make_edge(output_port<P::S0>(four_adder), input_port<0>(Sum));

                    make_edge(A[1].get_out(), input_port<P::A1>(four_adder));
                    make_edge(B[1].get_out(), input_port<P::B1>(four_adder));
                    make_edge(output_port<P::S1>(four_adder), input_port<1>(Sum));

                    make_edge(A[2].get_out(), input_port<P::A2>(four_adder));
                    make_edge(B[2].get_out(), input_port<P::B2>(four_adder));
                    make_edge(output_port<P::S2>(four_adder), input_port<2>(Sum));

                    make_edge(A[3].get_out(), input_port<P::A3>(four_adder));
                    make_edge(B[3].get_out(), input_port<P::B3>(four_adder));
                    make_edge(output_port<P::S3>(four_adder), input_port<3>(Sum));

                    make_edge(CarryIN.get_out(), input_port<P::CI>(four_adder));
                    make_edge(output_port<P::CO>(four_adder), CarryOUT.get_in());
                
                // Activate all switches at low state
                for (int i=0; i<4; ++i) {
                    A[i].activate();
                    B[i].activate();
                }
                CarryIN.activate();
                
                if (!silent) printf("1+0\n");
                A[0].flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == 1) && (CarryOUT.get_value() == low));
                
                if (!silent) printf("0+1\n");
                A[0].flip();
                B[0].flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == 1) && (CarryOUT.get_value() == low));
                
                if (!silent) printf("3+4\n");
                A[0].flip();
                A[1].flip();
                B[0].flip();
                B[2].flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == 7) && (CarryOUT.get_value() == low));
                
                if (!silent) printf("6+1\n");
                A[0].flip();
                A[2].flip();
                B[0].flip();
                B[2].flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == 7) && (CarryOUT.get_value() == low));
                
                if (!silent) printf("0+0+carry\n");
                A[1].flip();
                A[2].flip();
                B[0].flip();
                CarryIN.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == 1) && (CarryOUT.get_value() == low));
                
                if (!silent) printf("15+15+carry\n");
                A[0].flip();
                A[1].flip();
                A[2].flip();
                A[3].flip();
                B[0].flip();
                B[1].flip();
                B[2].flip();
                B[3].flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == 0xf) && (CarryOUT.get_value() == high));
                
                if (!silent) printf("8+8\n");
                A[0].flip();
                A[1].flip();
                A[2].flip();
                B[0].flip();
                B[1].flip();
                B[2].flip();
                CarryIN.flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == 0) && (CarryOUT.get_value() == high));
                
                if (!silent) printf("0+0\n");
                A[3].flip();
                B[3].flip();
                g.wait_for_all();
                if (!silent) Sum.display();
                if (!silent) CarryOUT.display();
                assert((Sum.get_value() == 0) && (CarryOUT.get_value() == low));
            }
#endif

            { // test D_latch
                D_latch my_d_latch(g);
                toggle D(g);
                pulse E(g, 500, 4); // clock changes every 500ms; stops after 4 changes
                led Q(g, " Q", verbose); // if true, LEDs print at every state change
                led notQ(g, "~Q", verbose);

                make_edge(D.get_out(), input_port<0>(my_d_latch)); 
                make_edge(E.get_out(), input_port<1>(my_d_latch));
                make_edge(output_port<0>(my_d_latch), Q.get_in());
                make_edge(output_port<1>(my_d_latch), notQ.get_in());

                D.activate();

                if (!silent) printf("Toggling D\n");
                E.activate();
                D.flip();
                g.wait_for_all();
                if (!silent && !verbose) { Q.display(); notQ.display(); }
                assert((Q.get_value() == high) && (notQ.get_value() == low));
                E.reset();
                
                if (!silent) printf("Toggling D\n");
                E.activate();
                D.flip();
                g.wait_for_all();
                if (!silent && !verbose) { Q.display(); notQ.display(); }
                assert((Q.get_value() == low) && (notQ.get_value() == high));
                E.reset();
                
                if (!silent) printf("Toggling D\n");
                E.activate();
                D.flip();
                g.wait_for_all();
                if (!silent && !verbose) { Q.display(); notQ.display(); }
                assert((Q.get_value() == high) && (notQ.get_value() == low));
                E.reset();
                
                if (!silent) printf("Toggling D\n");
                E.activate();
                D.flip();
                g.wait_for_all();
                if (!silent && !verbose) { Q.display(); notQ.display(); }
                assert((Q.get_value() == low) && (notQ.get_value() == high));
                E.reset();
                
                if (!silent) printf("Toggling D\n");
                E.activate();
                D.flip();
                g.wait_for_all();
                if (!silent && !verbose) { Q.display(); notQ.display(); }
                assert((Q.get_value() == high) && (notQ.get_value() == low));
            }
        }
        utility::report_elapsed_time((tbb::tick_count::now() - start).seconds());
        return 0;
    } catch(std::exception& e) {
        cerr<<"error occurred. error text is :\"" <<e.what()<<"\"\n";
        return 1;
    }
#else
    utility::report_skipped();
    return 0;
#endif // __TBB_FLOW_GRAPH_CPP11_FEATURES
}

