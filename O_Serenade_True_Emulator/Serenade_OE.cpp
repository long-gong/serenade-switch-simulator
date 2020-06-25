#ifndef SERENADE_OPTIMISTIC_HPP
#define SERENADE_OPTIMISTIC_HPP

#define Serenade "O"
#include "LWSConfig.h"

#include "lws.hpp"




std::vector<std::vector< KnowledgeSet > > knowledge_sets;
std::vector<std::vector< KnowledgeSet* > > B;
std::vector<std::vector<KnowledgeSet> > receive;

class SERENADE: virtual public Scheduler{
public:
    std::vector<int> r_matching;/* random matching */
    std::vector<int> s_output_matched;
    std::vector<int> merged_matching;
    std::vector<int> Ouroboros;
    int internal_counter;
    int Q_bound;

    SERENADE(int p, std::vector<int>& v):\
    Scheduler(p),  r_matching(p, 0), s_output_matched(p, 0),\
    merged_matching(p, 0), Ouroboros(v), internal_counter(0), Q_bound(std::numeric_limits<int>::max()) {

    }

    SERENADE(int p, std::vector<int>& v, int qb):\
    Scheduler(p),  r_matching(p, 0), s_output_matched(p, 0),\
    merged_matching(p, 0), Ouroboros(v), internal_counter(0), Q_bound(qb) {};

    virtual void run(lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments);
    virtual void reset() {
        internal_counter = 0;
    }
    void arrival_matching_greedy (lws_param_t &g_params, lws_status_t &g_status);
    void merge_emu(lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments);
    int binary_search(int i, int k, int wr, int wg, const int leader);
    bool knowledge_discovery_zero(int i, std::vector<int>& sigma, std::vector<int>& sigma_inverse, 
std::vector<int>& decesions, lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments);
    void knowledge_discovery_send(int i, int k,  
std::vector<int>& decesions, lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments);
    knowledge_discovery_receive(int i, int k, 
std::vector<int>& decesions, lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments);

};

void SERENADE::arrival_matching_greedy (lws_param_t &g_params, lws_status_t &g_status) {
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

void SERENADE::run(lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments){
    assert(g_params.N == port_num);

    if (internal_counter == 0) {/* initial */
        if (!UT::is_matching(g_status.S)) UT::identical_schedule(g_status.S);
    }

    /*! Step 1: greedily match in-out pairs which have arrival */
    arrival_matching_greedy(g_params, g_status);

    UT::fix_matching(r_matching); /*! round-robin match unmatched in-out pairs */

    // UT::O_merge(r_matching, g_status.S, merged_matching, g_status.Q,
    //           Ouroboros, Q_bound, g_instruments);
    merge_emu(g_params, g_status, g_instruments);
    g_status.S.swap(merged_matching);

    ++ internal_counter;
}


struct KnowledgeSet {
    int id;
    int wr;
    int wg;
    int leader;
};


void SERENADE::merge_emu(lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments){
    std::vector<int> sigma_green(port_num, LWS_UNMATCHED);
    std::vector<int> sigma(port_num, LWS_UNMATCHED);
    std::vector<int> sigma_inverse(port_num, LWS_UNMATCHED);
    for ( int i = 0;i < g_status.S.size();++ i ) {
        sigma_green[g_status.S[i]] = i;
    }
    for ( int i = 0;i < port_num;++ i ) {
        auto o = sigma_green[r_matching[i]];
        sigma[i] = o;
        sigma_inverse[o] = i;
    }

    if ( knowledge_sets.empty() ) {
        knowledge_sets.resize(port_num);
        B.resize(port_num, nullptr);
        receive.resize(port_num);
    }

    std::vector<int> decisions(port_num, -1);
    auto max_iter = int(std::ceil(std::log2(port_num));
    for ( int i = 0 ;i < port_num;++ i ) {
        knowledge_discovery_zero(i, sigma, sigma_inverse, decisions, g_params, g_status, g_instruments);
    }
    for ( int k = 1;k <= max_iter;++ k )

        for ( int i = 0 ;i < port_num;++ i ) {
            if ( decisions[i] == -1 ) {
                knowledge_discovery_send(i, k, decisions, g_params, g_status, g_instruments);
                knowledge_discovery_receive(i, k, decisions, g_params, g_status, g_instruments);
            }
        }
        
    }
    // binary search
    int k = max_iter;
    int n_cnt = 0;
    for ( int i = 0;i < port_num;++ i ) {
        if ( decisions[i] == -1 ) {
            auto ks& = knowledge_sets[i][2 * k + 1];
            auto l = ks.leader;
            if (  l == knowledge_sets[i][2 * k + 1].id) {
                decisions[l] = binary_search(i, k, ks.wr, ks.wg, l);
                ++ n_cnt;
            }
        }
    }
    for ( int i = 0;i < port_num;++ i ) {
        if ( decisions[i] == -1 ) {
            auto ks& = knowledge_sets[i][2 * k + 1];
            auto l = ks.leader;
            decisions[i] = decisions[l];
        }
    }
    

    std::fill(merged_matching.begin(), merged_matching.end(), -1);
    for ( int i = 0;i < port_num;++ i ) {
        merged_matching[i] = ( decisions[i] == 1 ? r_matching[i] : g_status.S[i] );
    }
}

int SERENADE::binary_search(int i, int k, int wr, int wg, const int leader){
    if ( i == leader ) {
        return ( wr >= wg ? 1: 0);
    } 
    int decision = -1;
    auto& ks = knowledge_sets[i][2 * k + 1 ];
    auto mid = ks.id;
    if ( mid == leader ) {
        wr -= ks.wr;
        wg -= ks.wg;
        decision = binary_search(mid, k - 1, wr, wg, leader);
    } else {
        if ( ks.leader == leader ) {
            decision = binary_search(i, k - 1, wr, wg, leader);
        } else {
            wg -= ks.wg;
            wr -= ks.wr;
            decision = binary_search(mid, k - 1, wr, wg, leader);
        }
    }
    return decision;
}

bool SERENADE::knowledge_discovery_zero(int i, std::vector<int>& sigma, std::vector<int>& sigma_inverse, 
std::vector<int>& decesions, lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments){
        auto i_d = sigma[i];
        auto i_u = sigma_inverse[i];
        knowledge_sets[i].emplace_back(i_d, g_status.Q[i][r_matching[i]], g_status.Q[i_d][r_matching[i]], -1);
        knowledge_sets[i].emplace_back(i_d, g_status.Q[i_u][r_matching[i_u]], g_status.Q[i][r_matching[i_u]], std::min(i_u, i));
        if ( i_d == i ) {
            decisions[i] = (knowledge_sets[i].back().wr >= knowledge_sets[i].back().wg ? 1:0);
            return true;
        } else if ( i_d == i_u ) {
            decisions[i] = (knowledge_sets[i][2 * k].wr + knowledge_sets[i][2 * k + 1].wr >= knowledge_sets[i][2 * k].wg + knowledge_sets[i][2 * k + 1].wg ? 1:0);
            return true;
        }
        return false;
}
void SERENADE::knowledge_discovery_send(int i, int k,  
std::vector<int>& decesions, lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments){

    auto i_d = knowledge_sets[i][2 * (k - 1)].id;
    auto i_u = knowledge_sets[i][2 * k - 1].id;
    receive[i_d].push_back(knowledge_sets[i][2 * k - 1]);
    receive[i_u].push_back(knowledge_sets[i][2 * (k - 1)]);

}

bool SERENADE::knowledge_discovery_receive(int i, int k, 
std::vector<int>& decesions, lws_param_t &g_params, lws_status_t &g_status, lws_inst_t &g_instruments){
    auto &ks = receive[i][1];
    if ( ks.leader == -1 ) {
        knowledge_sets[i].emplace_back(ks.id, ks.wr + knowledge_sets[i][2 * (k - 1)].wr, 
        knowledge_sets[i][2 * (k - 1)].wg + ks.wg, -1);
        knowledge_sets[i].emplace_back(receive[i][0].id, receive[i][0].wr + knowledge_sets[i][2 * k - 1].wr, 
        knowledge_sets[i][2 * k - 1].wg + receive[i][0].wg, std::min(receive[i][0].leader, knowledge_sets[i][2 * k - 1].leader));
    } else {
        knowledge_sets[i].emplace_back(receive[i][0].id, receive[i][0].wr + knowledge_sets[i][2 * k - 1].wr, 
        knowledge_sets[i][2 * k - 1].wg + receive[i][0].wg);  
        knowledge_sets[i].emplace_back(ks.id, ks.wr + knowledge_sets[i][2 * (k - 1)].wr, 
        knowledge_sets[i][2 * (k - 1)].wg + ks.wg, std::min(ks.leader, knowledge_sets[i][2 * k - 1].leader));     
    }

    auto &ks1 = knowledge_sets[i][2 * (k - 1)];
    auto &ks2 = knowledge_sets[i][2 * k - 1]
    auto i_d = ks1.id;
    auto i_u = ks2.id;
    if ( B[i_d] != nullptr ) {
        // rediscover
        if ( B[i_d]->leader == -1 ) {
            // same direction
            decisions[i] = ((ks1.wr - B[i_d]->wr) > (ks1.wg - B[i_d]->wg) ? 1 : 0);
        } else {
            // opposite direction
            decisions[i] = ((ks1.wr + B[i_d]->wr) > (ks1.wg + B[i_d]->wg) ? 1 : 0);
        }
        return true;
    } 
    if ( B[i_u] != nullptr ) {
        // rediscover
        if ( B[i_u]->leader != -1 ) {
            // same direction
            decisions[i] = ((ks2.wr - B[i_u]->wr) > (ks2.wg - B[i_u]->wg) ? 1 : 0);
        } else {
            // opposite direction
            decisions[i] = ((ks2.wr + B[i_u]->wr) > (ks2.wg + B[i_u]->wg) ? 1 : 0);
        }
        return true;        
    }
    if ( i_d == i_u ) {
        decisions[i] = ((ks1.wr + ks2.wr) > (ks1.wg + ks2.wg) ? 1 : 0);
        return true;
    }

    receive[i].clear();
    return false;
}
#endif