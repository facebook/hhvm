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

#ifndef __TBBexample_graph_logicsim_basics_H
#define __TBBexample_graph_logicsim_basics_H 1

#include <cstdio>
#include <string>
#include "tbb/atomic.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/tick_count.h"
#include "tbb/flow_graph.h"
#include "../../common/utility/utility.h"

#ifndef _WIN32
#include <sys/time.h>
#include <unistd.h>

void rt_sleep(int msec) {
    usleep(msec*1000);
}

#else //_WIN32

#undef OLDUNIXTIME
#undef STDTIME

#include <windows.h>

void rt_sleep(int msec) {
    Sleep(msec);
}
#endif  /*  _WIN32  */

using namespace std;
using namespace tbb;
using namespace tbb::flow;

typedef enum { low=0, high, undefined } signal_t;

template<int N> class gate;

template<>
class gate<1> : public composite_node< tuple< signal_t >, tuple< signal_t > > {
protected:
    typedef indexer_node<signal_t> input_port_t;
    typedef multifunction_node< input_port_t::output_type, tuple<signal_t> > gate_fn_t;
    typedef gate_fn_t::output_ports_type ports_type;
    typedef composite_node< tuple< signal_t >, tuple< signal_t > > base_type;
public:
    template <typename Body>
    gate(graph& g, Body b) : base_type(g), my_graph(g), in_ports(g), gate_fn(g, 1, b) {
        make_edge(in_ports, gate_fn);
        base_type::input_ports_type input_tuple(input_port<0>(in_ports));
        base_type::output_ports_type output_tuple(output_port<0>(gate_fn));
        base_type::set_external_ports(input_tuple, output_tuple);
        base_type::add_visible_nodes(in_ports, gate_fn);
    }
    virtual ~gate() {}
    gate& operator=(const gate& src) { return *this; }
protected:
    graph& my_graph;
private:
    input_port_t in_ports;
    gate_fn_t gate_fn;
};

template<>
class gate<2> : public composite_node< tuple< signal_t, signal_t >, tuple< signal_t > > {
protected:
    typedef indexer_node<signal_t,signal_t> input_port_t;
    typedef multifunction_node< input_port_t::output_type, tuple<signal_t> > gate_fn_t;
    typedef gate_fn_t::output_ports_type ports_type;
    typedef composite_node< tuple< signal_t, signal_t >, tuple< signal_t > > base_type;
public:
    template <typename Body>
    gate(graph& g, Body b) : base_type(g), my_graph(g), in_ports(g), gate_fn(g, 1, b) {
        make_edge(in_ports, gate_fn);
        base_type::input_ports_type input_tuple(input_port<0>(in_ports),input_port<1>(in_ports));
        base_type::output_ports_type output_tuple(output_port<0>(gate_fn));
        base_type::set_external_ports(input_tuple, output_tuple);
        base_type::add_visible_nodes(in_ports, gate_fn);
    }
    virtual ~gate() {}
    gate& operator=(const gate& src) { return *this; }
protected:
    graph& my_graph;
private:
    input_port_t in_ports;
    gate_fn_t gate_fn;
};

template<>
class gate<3> : public composite_node< tuple< signal_t, signal_t, signal_t >, tuple< signal_t > > {
protected:
    typedef indexer_node<signal_t, signal_t, signal_t> input_port_t;
    typedef multifunction_node< input_port_t::output_type, tuple<signal_t> > gate_fn_t;
    typedef gate_fn_t::output_ports_type ports_type;
    typedef composite_node< tuple< signal_t, signal_t, signal_t >, tuple< signal_t > > base_type;
public:
    template <typename Body>
    gate(graph& g, Body b) : base_type(g), my_graph(g), in_ports(g), gate_fn(g, 1, b) {
        make_edge(in_ports, gate_fn);
        base_type::input_ports_type input_tuple(input_port<0>(in_ports),input_port<1>(in_ports),input_port<2>(in_ports));
        base_type::output_ports_type output_tuple(output_port<0>(gate_fn));
        base_type::set_external_ports(input_tuple, output_tuple);
        base_type::add_visible_nodes(in_ports, gate_fn);
    }
    virtual ~gate() {}
    gate& operator=(const gate& src) { return *this; }
protected:
    graph& my_graph;
private:
    input_port_t in_ports;
    gate_fn_t gate_fn;
};

template<>
class gate<4> : public composite_node< tuple< signal_t, signal_t, signal_t, signal_t >, tuple< signal_t > > {
protected:
    typedef indexer_node<signal_t, signal_t, signal_t, signal_t> input_port_t;
    typedef multifunction_node< input_port_t::output_type, tuple<signal_t> > gate_fn_t;
    typedef gate_fn_t::output_ports_type ports_type;
    typedef composite_node< tuple< signal_t, signal_t, signal_t, signal_t >, tuple< signal_t > > base_type;
public:
    template <typename Body>
    gate(graph& g, Body b) : base_type(g), my_graph(g), in_ports(g), gate_fn(g, 1, b) {
        make_edge(in_ports, gate_fn);
        base_type::input_ports_type input_tuple(input_port<0>(in_ports),input_port<1>(in_ports),input_port<2>(in_ports), input_port<3>(in_ports)); 
        base_type::output_ports_type output_tuple(output_port<0>(gate_fn));
        base_type::set_external_ports(input_tuple, output_tuple);
        base_type::add_visible_nodes(in_ports, gate_fn);
    }
    virtual ~gate() {}
    gate& operator=(const gate& src) { return *this; }
protected:
    graph& my_graph;
private:
    input_port_t in_ports;
    gate_fn_t gate_fn;
};

// Input devices
class steady_signal {
    graph& my_graph;
    signal_t init_signal;
    write_once_node<signal_t> signal_node;
 public:
    steady_signal(graph& g, signal_t v) :
        my_graph(g), init_signal(v), signal_node(g) {}
    steady_signal(const steady_signal& src) : 
        my_graph(src.my_graph), init_signal(src.init_signal), 
        signal_node(src.my_graph) {}
    ~steady_signal() {}
    // Assignment is ignored
    steady_signal& operator=(const steady_signal& src) { return *this; }
    sender<signal_t>& get_out() { return signal_node; }
    void activate() { signal_node.try_put(init_signal); }
};

class pulse {
    class clock_body {
        size_t& ms;
        int& reps;
        signal_t val;
    public:
        clock_body(size_t& _ms, int& _reps) : ms(_ms), reps(_reps), val(low) {}
        bool operator()(signal_t& out) {
            rt_sleep((int)ms);
            if (reps>0) --reps;
            if (val==low) val = high;
            else val = low;
            out = val;
            return reps>0 || reps == -1;
        }
    };
    graph& my_graph;
    size_t ms, init_ms;
    int reps, init_reps;
    source_node<signal_t> clock_node;

public:
    pulse(graph& g, size_t _ms=1000, int _reps=-1) : 
        my_graph(g), ms(_ms), init_ms(_ms), reps(_reps), init_reps(_reps),
        clock_node(g, clock_body(ms, reps), false)
    {}
    pulse(const pulse& src) : 
        my_graph(src.my_graph), ms(src.init_ms), init_ms(src.init_ms),
        reps(src.init_reps), init_reps(src.init_reps), 
        clock_node(src.my_graph, clock_body(ms, reps), false)
    {}
    ~pulse() {}
    // Assignment changes the behavior of LHS to that of the RHS, but doesn't change owning graph
    pulse& operator=(const pulse& src) {
        ms = src.ms; init_ms = src.init_ms; reps = src.reps; init_reps = src.init_reps;
        return *this; 
    }
    sender<signal_t>& get_out() { return clock_node; }
    void activate() { clock_node.activate(); }
    void reset() { reps = init_reps; }
};

class push_button {
    graph& my_graph;
    overwrite_node<signal_t> push_button_node;
 public:
    push_button(graph& g) : my_graph(g), push_button_node(g) { 
        push_button_node.try_put(low);
    }
    push_button(const push_button& src) : 
        my_graph(src.my_graph), push_button_node(src.my_graph) { 
        push_button_node.try_put(low);
    }
    ~push_button() {}
    // Assignment is ignored
    push_button& operator=(const push_button& src) { return *this; }
    sender<signal_t>& get_out() { return push_button_node; }
    void press() { push_button_node.try_put(high); }
    void release() { push_button_node.try_put(low); }
};

class toggle {
    graph& my_graph;
    signal_t state;
    overwrite_node<signal_t> toggle_node;
 public:
    toggle(graph& g) : my_graph(g), state(undefined), toggle_node(g) {}
    toggle(const toggle& src) : my_graph(src.my_graph), state(undefined), 
                                toggle_node(src.my_graph) {}
    ~toggle() {}
    // Assignment ignored
    toggle& operator=(const toggle& src) { return *this; }
    sender<signal_t>& get_out() { return toggle_node; }
    void flip() { 
        if (state==high) state = low; 
        else state = high;
        toggle_node.try_put(state); 
    }
    void activate() { 
        state = low;
        toggle_node.try_put(state);
    }
};

// Basic gates
class buffer : public gate<1> {
    using gate<1>::my_graph;
    typedef gate<1>::ports_type ports_type;
    class buffer_body {
        signal_t state;
        bool touched;
    public:
        buffer_body() : state(undefined), touched(false) {}
        void operator()(const input_port_t::output_type &v, ports_type& p) { 
            if (!touched || state != cast_to<signal_t>(v)) {
                state = cast_to<signal_t>(v); 
                tbb::flow::get<0>(p).try_put(state); 
                touched = true;
            }
        }
    };
public: 
    buffer(graph& g) : gate<1>(g, buffer_body()) {}
    buffer(const buffer& src) : gate<1>(src.my_graph, buffer_body()) {}
    ~buffer() {}
};

class not_gate : public gate<1> {
    using gate<1>::my_graph;
    typedef gate<1>::ports_type ports_type;
    class not_body {
        signal_t port;
        bool touched;
    public:
    not_body() : port(undefined), touched(false) {}
        void operator()(const input_port_t::output_type &v, ports_type& p) {
            if (!touched || port != cast_to<signal_t>(v)) {
                port = cast_to<signal_t>(v); 
                signal_t state = low;
                if (port==low) state = high; 
                tbb::flow::get<0>(p).try_put(state);
                touched = true;
            }
        }
    };
 public: 
    not_gate(graph& g) : gate<1>(g, not_body()) {}
    not_gate(const not_gate& src) : gate<1>(src.my_graph, not_body()) {}
    ~not_gate() {}
};

template <int N>
class and_gate : public gate<N> {
    using gate<N>::my_graph;
    typedef typename gate<N>::ports_type ports_type;
    typedef typename gate<N>::input_port_t::output_type from_input;
    class and_body {
        signal_t *ports;
        signal_t state;
        bool touched;
    public:
        and_body() : state(undefined), touched(false) {
            ports = new signal_t[N];
            for (int i=0; i<N; ++i) ports[i] = undefined;
        }
        void operator()(const from_input& v, ports_type& p) {
            ports[v.tag()] = cast_to<signal_t>(v);
            signal_t new_state=high;
            size_t i=0;
            while (i<N) {
                if (ports[i] == low) { new_state = low; break; }
                else if (ports[i] == undefined && new_state != low) { new_state = undefined; }
                ++i;
            }
            if (!touched || state != new_state) {
                state = new_state;
                tbb::flow::get<0>(p).try_put(state);
                touched = true;
            }
        }
    };
 public:
    and_gate(graph& g) : gate<N>(g, and_body()) {}
    and_gate(const and_gate<N>& src) : gate<N>(src.my_graph, and_body()) {}
    ~and_gate() {}
};

template<int N>
class or_gate : public gate<N> {
    using gate<N>::my_graph;
    typedef typename gate<N>::ports_type ports_type;
    typedef typename gate<N>::input_port_t::output_type from_input;
    class or_body {
        signal_t *ports;
        signal_t state;
        bool touched;
    public:
        or_body() : state(undefined), touched(false) {
            ports = new signal_t[N];
            for (int i=0; i<N; ++i) ports[i] = undefined;
        }
        void operator()(const from_input& v, ports_type& p) {
            ports[v.tag()] = cast_to<signal_t>(v);
            signal_t new_state=low;
            size_t i=0;
            while (i<N) {
                if (ports[i] == high) { new_state = high; break; }
                else if (ports[i] == undefined && new_state != high) { new_state = undefined; }
                ++i;
            }
            if (!touched || state != new_state) {
                state = new_state;
                tbb::flow::get<0>(p).try_put(state);
                touched = true;
            }
        }
    };
public:
    or_gate(graph& g) : gate<N>(g, or_body()) {}
    or_gate(const or_gate& src) : gate<N>(src.my_graph, or_body()) {}
    ~or_gate() {}
};

template <int N>
class xor_gate : public gate<N> {
    using gate<N>::my_graph;
    typedef typename gate<N>::ports_type ports_type;
    typedef typename gate<N>::input_port_t input_port_t;
    class xor_body {
        signal_t *ports;
        signal_t state;
        bool touched;
    public:
        xor_body() : state(undefined), touched(false) {
            ports = new signal_t[N];
            for (int i=0; i<N; ++i) ports[i] = undefined;
        }
        void operator()(const typename input_port_t::output_type &v, ports_type& p) {
            ports[v.tag()] = cast_to<signal_t>(v);
            signal_t new_state=low;
            size_t i=0, highs=0;
            while (i<N) {
                if (ports[i] == undefined) { new_state = undefined; }  
                else if (ports[i] == high && new_state == low) { new_state = high; ++highs; }
                else if (ports[i] == high && highs > 0) { new_state = low; break; }
                else if (ports[i] == high ) { ++highs; }
                ++i;
            }
            if (!touched || state != new_state) {
                state = new_state;
                tbb::flow::get<0>(p).try_put(state);
                touched = true;
            }
        }
    };
 public:
    xor_gate(graph& g) : gate<N>(g, xor_body()) {}
    xor_gate(const xor_gate& src) : gate<N>(src.my_graph, xor_body()) {}
    ~xor_gate() {}
};

template <int N>
class nor_gate : public gate<N> {
    using gate<N>::my_graph;
    typedef typename gate<N>::ports_type ports_type;
    typedef typename gate<N>::input_port_t input_port_t;
    class nor_body {
        signal_t *ports;
        signal_t state;
        bool touched;
    public:
        nor_body() : state(undefined), touched(false) {
            ports = new signal_t[N];
            for (int i=0; i<N; ++i) ports[i] = undefined;
        }
        void operator()(const typename input_port_t::output_type &v, ports_type& p) {
            ports[v.tag()] = cast_to<signal_t>(v);
            signal_t new_state=low;
            size_t i=0;
            while (i<N) {
                if (ports[i] == high) { new_state = high; break; }
                else if (ports[i] == undefined && new_state != high) { new_state = undefined; }
                ++i;
            }
            if (new_state == high) new_state = low;
            else if (new_state == low) new_state = high;
            if (!touched || state != new_state) {
                state = new_state;
                tbb::flow::get<0>(p).try_put(state);
                touched = true;
            }
        }
    };
 public:
    nor_gate(graph& g) : gate<N>(g, nor_body()) {}
    nor_gate(const nor_gate& src) : gate<N>(src.my_graph, nor_body()) {}
    ~nor_gate() {}
};

// Output devices
class led {
    class led_body {
        signal_t &state;
        string &label;
        bool report_changes;
        bool touched;
    public:
        led_body(signal_t &s, string &l, bool r) :
            state(s), label(l), report_changes(r), touched(false)
        {}
        continue_msg operator()(signal_t b) {
            if (!touched || b!=state) {
                state = b;
                if (state != undefined && report_changes) {
                    if (state) printf("%s: (*)\n", label.c_str());
                    else printf("%s: ( )\n", label.c_str());
                }
                touched = false;
            }
            return continue_msg();
        }
    };
    graph& my_graph;
    string label;
    signal_t state;
    bool report_changes;
    function_node<signal_t, continue_msg> led_node;
 public:
    led(graph& g, string l, bool rc=false) : my_graph(g), label(l), state(undefined), 
                                             report_changes(rc), 
                                             led_node(g, 1, led_body(state, label, report_changes))
    {}
    led(const led& src) : my_graph(src.my_graph), label(src.label), state(undefined), 
                          report_changes(src.report_changes), 
                          led_node(src.my_graph, 1, led_body(state, label, report_changes)) 
    {}
    ~led() {}
    // Assignment changes the behavior of LHS to that of the RHS, but doesn't change owning graph
    // state is set to undefined so that next signal changes it
    led& operator=(const led& src) { 
        label = src.label; state = undefined; report_changes = src.report_changes; 
        return *this;
    }
    receiver<signal_t>& get_in() { return led_node; }
    void display() { 
        if (state == high) printf("%s: (*)\n", label.c_str());
        else if (state == low) printf("%s: ( )\n", label.c_str());
        else printf("%s: (u)\n", label.c_str());
    }
    signal_t get_value() { return state; }
};

class digit : public gate<4> {
    using gate<4>::my_graph;
    typedef gate<4>::ports_type ports_type;
    typedef gate<4>::input_port_t input_port_t;
    class digit_body {
        signal_t ports[4];
        static const int N = 4;
        unsigned int &state;
        string &label;
        bool& report_changes;
    public:
        digit_body(unsigned int &s, string &l, bool& r) : state(s), label(l), report_changes(r) {
            for (int i=0; i<N; ++i) ports[i] = undefined;
        }
        void operator()(const input_port_t::output_type& v, ports_type& p) {
            unsigned int new_state = 0;
            ports[v.tag()] = cast_to<signal_t>(v);
            if (ports[0] == high) ++new_state;
            if (ports[1] == high) new_state += 2;
            if (ports[2] == high) new_state += 4;
            if (ports[3] == high) new_state += 8;
            if (state != new_state) {
                state = new_state;
                if (report_changes) {
                    printf("%s: %x\n", label.c_str(), state);
                }
            }
        }
    };
    string label;
    unsigned int state;
    bool report_changes;
 public:
    digit(graph& g, string l, bool rc=false) : 
        gate<4>(g, digit_body(state, label, report_changes)), 
        label(l), state(0), report_changes(rc) {}
    digit(const digit& src) : 
        gate<4>(src.my_graph, digit_body(state, label, report_changes)), 
        label(src.label), state(0), report_changes(src.report_changes) {}
    ~digit() {}
    // Assignment changes the behavior of LHS to that of the RHS, but doesn't change owning graph.
    // state is reset as in constructors
    digit& operator=(const digit& src) { 
        label = src.label; state = 0; report_changes = src.report_changes; 
        return *this;
    }
    void display() { printf("%s: %x\n", label.c_str(), state); }
    unsigned int get_value() { return state; }
};

#endif /* __TBBexample_graph_logicsim_basics_H */
