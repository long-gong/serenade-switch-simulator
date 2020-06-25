#ifndef ISLIP_HPP
#define ISLIP_HPP

#include "lws.hpp"

#define ISLIP_DEF_ACCEPT 0
#define ISLIP_DEF_GRANT 0



class iSLIP: virtual public iScheduler{
public:
    std::vector<int> accept;
    std::vector<int> grant;
    iSLIP(int port_num):\
        Scheduler(port_num), iScheduler(port_num), accept(port_num, ISLIP_DEF_ACCEPT), grant(port_num, ISLIP_DEF_GRANT){

    }
    void run(lws_param_t &g_params, lws_status_t &g_status);
    void reset() {
        std::fill(accept.begin(), accept.end(), ISLIP_DEF_ACCEPT);
        std::fill(grant.begin(), grant.end(), ISLIP_DEF_GRANT);
    }
    void one_round_(std::vector<std::vector<int> >& VOQs, std::vector<int>& S,\
        bool is_first_round);
};


void iSLIP::run(lws_param_t &g_params, lws_status_t &g_status){
    int N = accept.size();
    assert(g_params.N == N);

    int t, i;
    bool is_first_round;
    std::fill(input_matched.begin(), input_matched.end(), LWS_UNMATCHED);
    std::fill(output_matched.begin(), output_matched.end(), LWS_UNMATCHED);
   
    for (int i = 0;i < N;++ i) g_status.S[i] = LWS_UNMATCHED;

    for (t = 0;t < rounds + 1;++ t) {
        is_first_round = (t == 0);
        one_round_(g_status.Q, g_status.S, is_first_round);
    }
    /* comment the following line */
    assert(UT::is_partial_matching(g_status.S));

}

void iSLIP::one_round_(std::vector<std::vector<int> >& VOQs, std::vector<int>& S,\
bool is_first_round) {
    int N = S.size();
    int i, k, c;
    bool found;

    /* Request & Grant */
    for (k = 0; k < N; ++k) {/* each output port */
        check[k] = LWS_UNMATCHED;/* mark  as no requests */
        if (output_matched[k] == LWS_UNMATCHED) { /* unmatched ====> check requests */
            for (found = false, c = 0; c < N && !found; ++c) {
                i = (c + grant[k]) % N;/* get grant pointer */
                if ((input_matched[i] == LWS_UNMATCHED) && VOQs[i][k] > 0) { /* i not matched */
                    found = true;
                    check[k] = i; /* grant to i */
                }
            }
        }
    }
    /* Accept */
    for (i = 0; i < N; ++i) {/* each input port */
        if (input_matched[i] == LWS_UNMATCHED) {/* not matched */
            for (found = false, c = 0; c < N && !found; ++ c) {
                k = (c + accept[i]) % N;
                if (check[k] == i) {
                    found = true;
                    S[i] = k;
                    input_matched[i] = k;
                    output_matched[k] = i;
                    if (is_first_round) {
                        grant[k] = (i + 1) % N;
                        accept[i] = (k + 1) % N;
                    }
                }
            }
        }
    }
}

#endif