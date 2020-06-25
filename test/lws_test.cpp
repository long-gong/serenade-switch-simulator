#include <gtest/gtest.h>

// the following must before the include of "lws.hpp"
#define Serenade "Test"
#include "LWSConfig.h"

#include "lws.hpp"



// dummy test
TEST(DummyTest, dummytest1) {
    EXPECT_EQ(0, 0);
}

// test default options
TEST(TestParam, TestDefaultOptions) {
    int argc = 3;
    char *argv[] = {(char *)"TestDefaultOptions", (char *)"-n", (char *)"TestDefaultOptions"};
    lws_param_t params(argc, argv);
    EXPECT_EQ(params.name, "TestDefaultOptions");
    EXPECT_EQ(LWS_DELAY_VS_LOAD, params.type);
    EXPECT_EQ(LWS_ALL_MODELS, params.model_option);
    EXPECT_TRUE(params.model_name.empty());
    EXPECT_EQ(LWS_BERNOULLI_IID, params.inject_option);
    EXPECT_EQ(LWS_DEF_PORT_NUM, params.N);
    EXPECT_EQ(LWS_DEF_FRAMES * LWS_DEF_PORT_NUM * LWS_DEF_PORT_NUM, params.MAX_M);
    EXPECT_TRUE(params.seed > 0);
    EXPECT_EQ(LWS_DEF_VERBOSITY_LEVEL, params.verbosity);
//    EXPECT_EQ(0, params.average_burst_length);
    EXPECT_TRUE(params.abls.empty());
//    EXPECT_EQ(0, params.prob_on);
//    EXPECT_EQ(0, params.prob_off);
    EXPECT_EQ(params.loads, std::vector<double>({0.99, 0.95, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1}));
//    EXPECT_EQ(0, params.load);
    EXPECT_EQ(fmt::format("../Ouroboros/n={}.txt", LWS_DEF_PORT_NUM), params.Ouroboros_filename);
//    EXPECT_EQ("../results/-0-0-0.dat", params.Ouroboros_stats_filename);
    EXPECT_EQ(std::numeric_limits<int>::max(), params.Q_bound);
}

// test customized options 1: delay vs loads
TEST(TestParam, TestCustomizedOptions1) {
    char *argv[] = {
            (char *)"TestCustomizedOptions",
            (char *)"-p", (char *)"128",
            (char *)"-l", (char *)"0.9",
            (char *)"-l", (char *)"0.8",
            (char *)"-m", (char *)"100",
            (char *)"-B", (char *)"1000"
    };
    int argc = 11;
    lws_param_t params(argc, argv);
    std::cout << (params.to_json()).dump(4) << std::endl;

    EXPECT_EQ(params.name, "Simulator");
    EXPECT_EQ(LWS_DELAY_VS_LOAD, params.type);
    EXPECT_EQ(LWS_ALL_MODELS, params.model_option);
    EXPECT_TRUE(params.model_name.empty());
    EXPECT_EQ(LWS_BERNOULLI_IID, params.inject_option);
    EXPECT_EQ(128, params.N);
    EXPECT_EQ(100 * 128 * 128, params.MAX_M);
    EXPECT_TRUE(params.seed > 0);
    EXPECT_EQ(LWS_DEF_VERBOSITY_LEVEL, params.verbosity);
//    EXPECT_EQ(0, params.average_burst_length);
    EXPECT_TRUE(params.abls.empty());
//    EXPECT_EQ(0, params.prob_on);
//    EXPECT_EQ(0, params.prob_off);
    EXPECT_EQ(params.loads, std::vector<double>({0.9, 0.8}));
//    EXPECT_EQ(0, params.load);
    EXPECT_EQ(fmt::format("../Ouroboros/n={}.txt", 128), params.Ouroboros_filename);
//    EXPECT_EQ("../results/-0-0-0.dat", params.Ouroboros_stats_filename);
    EXPECT_EQ(1000, params.Q_bound);
}

// test customized options 1: delay vs burst sizes
TEST(TestParam, TestCustomizedOptions2) {
    char *argv[] = {
            (char *)"TestCustomizedOptions",
            (char *)"-p", (char *)"128",
            (char *)"-l", (char *)"0.9",
            (char *)"-l", (char *)"0.8",
            (char *)"-m", (char *)"100",
            (char *)"-B", (char *)"1000",
            (char *)"-b", (char *)"2",
            (char *)"-b", (char *)"4"
    };
    int argc = 15;
    lws_param_t params(argc, argv);
    std::cout << (params.to_json()).dump(4) << std::endl;

    EXPECT_EQ(params.name, "Simulator");
    EXPECT_EQ(LWS_DELAY_VS_BURST_SIZE, params.type);
    EXPECT_EQ(LWS_ALL_MODELS, params.model_option);
    EXPECT_TRUE(params.model_name.empty());
    EXPECT_EQ(LWS_ON_OFF_BURST, params.inject_option);
    EXPECT_EQ(128, params.N);
    EXPECT_EQ(100 * 128 * 128, params.MAX_M);
    EXPECT_TRUE(params.seed > 0);
    EXPECT_EQ(LWS_DEF_VERBOSITY_LEVEL, params.verbosity);
//    EXPECT_EQ(0, params.average_burst_length);
    EXPECT_EQ(params.abls, std::vector<double>({2, 4}));
//    EXPECT_EQ(0, params.prob_on);
//    EXPECT_EQ(0, params.prob_off);
    EXPECT_EQ(params.loads, std::vector<double>({0.9, 0.8}));
//    EXPECT_EQ(0, params.load);
    EXPECT_EQ(fmt::format("../Ouroboros/n={}.txt", 128), params.Ouroboros_filename);
//    EXPECT_EQ("../results/-0-0-0.dat", params.Ouroboros_stats_filename);
    EXPECT_EQ(1000, params.Q_bound);
}


// test instrument
TEST(TestInstrument, testAll) {
    int pn = 64;
    lws_inst_t instruments(pn);
    EXPECT_EQ(0, instruments.max_delay);
    EXPECT_EQ(0, instruments.mean_delay);
    EXPECT_EQ(0, instruments.mean_delay_count);
    EXPECT_EQ(0, instruments.sum_delay);
    EXPECT_EQ(0, instruments.T_arr);
    EXPECT_EQ(0, instruments.T_dep);

    EXPECT_EQ(0, instruments.C_counter_O);
    EXPECT_EQ(0, instruments.C_counter_T);
    EXPECT_EQ(0, instruments.C_counter_W);

    EXPECT_EQ(0, instruments.W_counter_O);
    EXPECT_EQ(0, instruments.W_counter_T);
    EXPECT_EQ(0, instruments.W_counter_W);

    EXPECT_EQ(std::vector<double>(pn, 0), instruments.C_size_hist);

    EXPECT_EQ(pn + 7, instruments.tn);

    std::cout << instruments.to_string(true) << std::endl;
    std::cout << instruments.to_string() << std::endl;


    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, 10000);

    instruments.max_delay += distribution(generator);
    instruments.mean_delay += distribution(generator);
    instruments.mean_delay_count += distribution(generator);
    instruments.sum_delay += distribution(generator);
    instruments.T_arr += distribution(generator);
    instruments.T_dep += distribution(generator);

    instruments.C_counter_O += distribution(generator);
    instruments.C_counter_T += distribution(generator);
    instruments.C_counter_W += distribution(generator);

    instruments.W_counter_O += distribution(generator);
    instruments.W_counter_T += distribution(generator);
    instruments.W_counter_W += distribution(generator);

    for (int i = 0;i < instruments.C_size_hist.size();++ i)
        instruments.C_size_hist[i] += distribution(generator);

    std::cout << instruments.to_string(true) << std::endl;
    std::cout << instruments.to_string() << std::endl;

    instruments.reset();

    EXPECT_EQ(0, instruments.max_delay);
    EXPECT_EQ(0, instruments.mean_delay);
    EXPECT_EQ(0, instruments.mean_delay_count);
    EXPECT_EQ(0, instruments.sum_delay);
    EXPECT_EQ(0, instruments.T_arr);
    EXPECT_EQ(0, instruments.T_dep);

    EXPECT_EQ(0, instruments.C_counter_O);
    EXPECT_EQ(0, instruments.C_counter_T);
    EXPECT_EQ(0, instruments.C_counter_W);

    EXPECT_EQ(0, instruments.W_counter_O);
    EXPECT_EQ(0, instruments.W_counter_T);
    EXPECT_EQ(0, instruments.W_counter_W);

    EXPECT_EQ(std::vector<double>(pn, 0), instruments.C_size_hist);

    EXPECT_EQ(pn + 6, instruments.tn);

    std::cout << instruments.to_string(true) << std::endl;
    std::cout << instruments.to_string() << std::endl;

}


// test status class
TEST(TestStatus, testAll) {
    int pn = 8;
    lws_status_t status(pn);

    std::vector<int> S(pn, 0);
    for (int i = 0;i < S.size();++ i) S[i] = i;

    EXPECT_EQ(std::vector<int>(pn, LWS_NOARRIVAL), status.A);
    EXPECT_EQ(0, status.cur_time);
    EXPECT_EQ(std::vector<std::vector<int> >(pn, std::vector<int>(pn, 0)), status.Q);
    EXPECT_EQ(S, status.S);
    EXPECT_EQ(pn, status.port_num);
    EXPECT_EQ(pn, status.B.size());
    for (int i = 0;i < pn;++ i) EXPECT_EQ(pn, status.B[i].size());

    std::cout << status.to_json().dump(4) << std::endl;

    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0,1.0);

    for (int i = 0;i < pn;++ i){
        if (distribution(generator) < 0.5) status.S[i] = 1;
        else status.S[i] = 0;
        if (distribution(generator) < 0.4) status.A[i] = 1;
        else status.A[i] = LWS_NOARRIVAL;
        for (int k = 0;k < pn;++ k)
            if (distribution(generator) < 0.6) status.Q[i][k] += int(1000 * distribution(generator));
        status.cur_time += (int)(5656 * distribution(generator));
    }
    std::cout << status.to_json().dump(4) << std::endl;

    status.reset();

    EXPECT_EQ(std::vector<int>(pn, LWS_NOARRIVAL), status.A);
    EXPECT_EQ(0, status.cur_time);
    EXPECT_EQ(std::vector<std::vector<int> >(pn, std::vector<int>(pn, 0)), status.Q);
    EXPECT_EQ(S, status.S);
    EXPECT_EQ(pn, status.port_num);
    EXPECT_EQ(pn, status.B.size());
    std::cout << status.to_json().dump(4) << std::endl;


    lws_status_t status2(pn, true);
    EXPECT_EQ(std::vector<int>(pn, LWS_NOARRIVAL), status2.A);
    EXPECT_EQ(0, status2.cur_time);
    EXPECT_EQ(std::vector<std::vector<int> >(pn, std::vector<int>(pn, 0)), status2.Q);
    EXPECT_EQ(S, status2.S);
    EXPECT_EQ(pn, status2.port_num);
    EXPECT_TRUE(status2.B.empty());

    for (int i = 0;i < pn;++ i){
        if (distribution(generator) < 0.5) status2.S[i] = 1;
        else status2.S[i] = 0;
        if (distribution(generator) < 0.4) status2.A[i] = 1;
        else status.A[i] = LWS_NOARRIVAL;
        for (int k = 0;k < pn;++ k)
            if (distribution(generator) < 0.6) status2.Q[i][k] += int(1000 * distribution(generator));
        status2.cur_time += (int)(5656 * distribution(generator));
    }
    std::cout << status2.to_json().dump(4) << std::endl;

    status2.reset();

    EXPECT_EQ(std::vector<int>(pn, LWS_NOARRIVAL), status2.A);
    EXPECT_EQ(0, status2.cur_time);
    EXPECT_EQ(std::vector<std::vector<int> >(pn, std::vector<int>(pn, 0)), status2.Q);
    EXPECT_EQ(S, status2.S);
    EXPECT_EQ(pn, status2.port_num);
    EXPECT_TRUE(status2.B.empty());

    std::cout << status2.to_json().dump(4) << std::endl;


}


// test switchcore class
TEST(TestSwitchCore, testAll) {
    char *argv[] = {
            (char *)"TestCustomizedOptions",
            (char *)"-p", (char *)"128",
            (char *)"-l", (char *)"0.9",
            (char *)"-l", (char *)"0.8",
            (char *)"-m", (char *)"100",
            (char *)"-B", (char *)"1000"
    };
    int argc = 11;
    LWS_SwitchCore lsc(argc, argv);

    lsc.print_all();

    EXPECT_EQ("Simulator", lsc.params.name);
    EXPECT_EQ(LWS_DELAY_VS_LOAD, lsc.params.type);
    EXPECT_EQ(LWS_ALL_MODELS, lsc.params.model_option);
    EXPECT_TRUE(lsc.params.model_name.empty());
    EXPECT_EQ(LWS_BERNOULLI_IID, lsc.params.inject_option);
    EXPECT_EQ(128, lsc.params.N);
    EXPECT_EQ(100 * 128 * 128, lsc.params.MAX_M);
    EXPECT_TRUE(lsc.params.seed > 0);
    EXPECT_EQ(LWS_DEF_VERBOSITY_LEVEL, lsc.params.verbosity);
//    EXPECT_EQ(0, params.average_burst_length);
    EXPECT_TRUE(lsc.params.abls.empty());
//    EXPECT_EQ(0, params.prob_on);
//    EXPECT_EQ(0, params.prob_off);
    EXPECT_EQ(std::vector<double>({0.9, 0.8}), lsc.params.loads);
//    EXPECT_EQ(0, params.load);
    EXPECT_EQ(fmt::format("../Ouroboros/n={}.txt", 128), lsc.params.Ouroboros_filename);
//    EXPECT_EQ("../results/-0-0-0.dat", params.Ouroboros_stats_filename);
    EXPECT_EQ(1000, lsc.params.Q_bound);


    EXPECT_EQ(0, lsc.instruments.max_delay);
    EXPECT_EQ(0, lsc.instruments.mean_delay);
    EXPECT_EQ(0, lsc.instruments.mean_delay_count);
    EXPECT_EQ(0, lsc.instruments.sum_delay);
    EXPECT_EQ(0, lsc.instruments.T_arr);
    EXPECT_EQ(0, lsc.instruments.T_dep);

    EXPECT_EQ(0, lsc.instruments.C_counter_O);
    EXPECT_EQ(0, lsc.instruments.C_counter_T);
    EXPECT_EQ(0, lsc.instruments.C_counter_W);

    EXPECT_EQ(0, lsc.instruments.W_counter_O);
    EXPECT_EQ(0, lsc.instruments.W_counter_T);
    EXPECT_EQ(0, lsc.instruments.W_counter_W);

    EXPECT_EQ(std::vector<double>(lsc.params.N, 0), lsc.instruments.C_size_hist);

    EXPECT_EQ(lsc.params.N + 7, lsc.instruments.tn);

    std::vector<int> S(lsc.params.N, 0);
    for (int i = 0;i < S.size();++ i) S[i] = i;

    EXPECT_EQ(std::vector<int>(lsc.params.N, LWS_NOARRIVAL), lsc.status.A);
    EXPECT_EQ(0, lsc.status.cur_time);
    EXPECT_EQ(std::vector<std::vector<int> >(lsc.params.N, std::vector<int>(lsc.params.N, 0)), lsc.status.Q);
    EXPECT_EQ(S, lsc.status.S);
    EXPECT_EQ(lsc.params.N, lsc.status.port_num);
    EXPECT_EQ(lsc.params.N, lsc.status.B.size());
    for (int i = 0;i < lsc.params.N;++ i) EXPECT_EQ(lsc.params.N, lsc.status.B[i].size());

    lsc.show(std::cout);

}
GTEST_API_ int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}