//
// C-Serenade for the sake of stability proof
//
// Created by Long Gong on 12/4/16.
//
#ifndef SERENADE_CONSERVATIVE_HPP
#define SERENADE_CONSERVATIVE_HPP

#define Serenade "C"

#include <fstream>
#include "LWSConfig.h"

#include "lws.hpp"


// global variables that can only be accessed in this file
static std::default_random_engine generator;
static std::uniform_real_distribution<double> distribution(0.0,1.0);

/*
 * added for ton 2018
 */
static const size_t  MS = (1 << 10);
static std::vector<int> SW(MS, 0);
static std::vector<int> TW(MS, 0);
static std::vector<int> ES(MS, 0);
static std::string fn_bk("");
static size_t CIndex = 0;

class Serenade_Conservative: virtual public Scheduler{
public:
    std::vector<int> r_matching;/* random matching */
    std::vector<int> s_output_matched;
    std::vector<int> merged_matching;
    std::vector<int> Ouroboros;
    std::vector<int> BSIT;
    int internal_counter;
    double frequency;

    Serenade_Conservative(int p, std::vector<int>& v, std::vector<int>& it, double freq):\
    Scheduler(p),  r_matching(p, 0), s_output_matched(p, 0),\
    merged_matching(p, 0), Ouroboros(v), BSIT(it),
    internal_counter(0), frequency(freq) {};

    virtual void run(lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments);
    virtual void reset() {
        if (!fn_bk.empty()){
            std::ofstream ofs(fn_bk, std::ios::app);
            if (ofs.fail()){// open failed
                std::cerr << "Open file " << fn_bk << " failed" << std::endl;
                exit(1);
            }
            for (size_t i = 0;i < CIndex; ++ i){
                ofs << ES[i] << "," << SW[i] << "," << TW[i] << "\n";
            }
            ofs.close();
        }
        CIndex = 0;
        internal_counter = 0;
    }
    void arrival_matching_greedy (lws_param_t &g_params, lws_status_t &g_status);
};

void Serenade_Conservative::arrival_matching_greedy (lws_param_t &g_params, lws_status_t &g_status) {
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

void Serenade_Conservative::run(lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments){
    assert(g_params.N == port_num);

    if (internal_counter == 0) {/* initial */
        if (!UT::is_matching(g_status.S)) UT::identical_schedule(g_status.S);
    }

    /*! Step 1: greedily match in-out pairs which have arrival */
    arrival_matching_greedy(g_params, g_status);

    UT::fix_matching(r_matching); /*! round-robin match unmatched in-out pairs */

    // pay attention matching from last time slot MUST be the first argument
//    if (distribution(generator) <= 1.0 / frequency)
//        UT::E_merge_v1(g_status.S, r_matching, merged_matching, g_status.Q, BSIT, g_instruments); // E-Serenade
//    else
        UT::C_merge_v2(g_status.S, r_matching, merged_matching, g_status.Q, Ouroboros, g_instruments);
        g_status.S.swap(merged_matching); // C-Serenade

    // for ton 2018
    int weight_sched = 0;
    int effective_size = 0;
    for (size_t i = 0;i < port_num;++ i){
        auto j = g_status.S[i];
        assert(j >= 0 && j < port_num);
        weight_sched += g_status.Q[i][j];
        if (g_status.Q[i][j] > 0) ++ effective_size;
    }
    SW[CIndex] = weight_sched;
    ES[CIndex] = effective_size;
    int weight_total = 0;
    for (size_t i = 0;i < port_num;++ i){
        for (size_t j = 0;j < port_num;++ j){
            weight_total += g_status.Q[i][j];
        }
    }
    TW[CIndex] = weight_total;
    ++ CIndex;
    if (CIndex == MS){
        std::cout << "\n\nWrite results to file ...\n\n";
        std::string fn("weights_p" + std::to_string(port_num) + "_l" +
                       std::to_string(g_params.load) + "_i" +
                       std::to_string(g_params.model_option) + ".txt");
        std::ofstream ofs(fn, std::ios::app);
        if (ofs.fail()){// open failed
            std::cerr << "Open file " << fn << " failed" << std::endl;
            exit(1);
        }
        for (size_t i = 0;i < MS; ++ i){
            ofs << ES[i] << "," << SW[i] << "," << TW[i] << "\n";
        }
        CIndex = 0;
        ofs.close();
        fn_bk.clear();
        fn_bk.insert(fn_bk.begin(), fn.begin(), fn.end());
    }

    ++ internal_counter;
}
#endif