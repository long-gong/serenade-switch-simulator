//
// Serena
//
// Created by Long Gong on 10/1/16.
//
#ifndef SERENADE_OPTIMISTIC_HPP
#define SERENADE_OPTIMISTIC_HPP

#define Serenade "E"
#include "LWSConfig.h"

#include "lws.hpp"



class E_Serenade: virtual public Scheduler{
public:
    std::vector<int> r_matching;/* random matching */
    std::vector<int> s_output_matched;
    std::vector<int> merged_matching;

    std::vector<int> BSIT;
    int internal_counter;

    E_Serenade(int p, std::vector<int>& it):\
    Scheduler(p),  r_matching(p, 0), s_output_matched(p, 0),\
    merged_matching(p, 0), BSIT(it),
    internal_counter(0) {

    };


    virtual void run(lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments);

    virtual void reset() {
        internal_counter = 0;
    }

    void arrival_matching_greedy (lws_param_t &g_params, lws_status_t &g_status);
};

void E_Serenade::arrival_matching_greedy (lws_param_t &g_params, lws_status_t &g_status) {
    assert(g_params.N == r_matching.size());
    /* pay attention */
    std::fill(r_matching.begin(), r_matching.end(), LWS_UNMATCHED);
    std::fill(s_output_matched.begin(), s_output_matched.end(), LWS_UNMATCHED);

    int i, k, pre_i;
    for (i = 0;i < g_params.N;++ i) {
        k = g_status.A[i];
        if (k != -1) {/*! arrival from i to k */
            assert(k >= 0 && k < g_params.N);
            if (s_output_matched[k] == LWS_UNMATCHED) /* unmatched before */
                s_output_matched[k] = i;
            else {/*! multiple inputs want to go */
                pre_i = s_output_matched[k];
                assert(pre_i >= 0 && pre_i < g_params.N);
                if (g_status.Q[i][k] > g_status.Q[pre_i][k]) {/*! current match is `better` */
                    s_output_matched[k] = i;
                }
            }
        }
    }

    for (k = 0;k < g_params.N;++ k) {
        i = s_output_matched[k];
        if (i != LWS_UNMATCHED) r_matching[i] = k;
    }
}

void E_Serenade::run(lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments){
    assert(g_params.N == port_num);

    if (internal_counter == 0) {/* initial */
        if (!UT::is_matching(g_status.S)) UT::identical_schedule(g_status.S);
    }

    /*! Step 1: greedily match in-out pairs which have arrival */
    arrival_matching_greedy(g_params, g_status);

    UT::fix_matching(r_matching); /*! round-robin match unmatched in-out pairs */

    UT::E_merge_v1(g_status.S, r_matching, merged_matching, g_status.Q, BSIT, g_instruments);

    g_status.S.swap(merged_matching);

    ++ internal_counter;
}
#endif