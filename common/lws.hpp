//
// Created by Long Gong on 9/27/16.
//

#ifndef LWS_H
#define LWS_H



#if defined(Serenade) && defined(LWS_VERSION)
#include <fmt/format.h>
#else
// for backward comp
#include <fstream>
#endif


#include <iostream>
#include <iomanip>
#include <string>

#include <cmath>
#include <queue> // for FIFO queue
#include <vector>
#include <initializer_list>
#include <memory>
#include <cassert>
#include <limits>
#include <chrono>
#include "cxxopts.hpp"
#include "json.hpp"
#include "random_variable.hpp"

#include <hdr/hdr_histogram.h>

#include <limits>




/* ---------------------------------------------------------------------------------- */
/*                                  Macros                                            */
/* ---------------------------------------------------------------------------------- */


/*
 *TODO: change macros to enum
 */
/* special constant */
#define LWS_NOARRIVAL -1
#define LWS_UNMATCHED -1
#define LWS_NODEPARTURE -1
#define LWS_MATCHED 1


/* inject options */
#define LWS_BERNOULLI_IID 0
#define LWS_ON_OFF_BURST 1

/* traffic matrix options */
#define LWS_UNIFORM 0
#define LWS_LOG_DIAGONAL 1
#define LWS_QUASI_DIAGONAL 2
#define LWS_DIAGONAL 3
#define LWS_ALL_MODELS -1


/* logging options */
#define LWS_SCREEN_ONLY 0
#define LWS_FILE_ONLY 1
#define LWS_BOTH 2
#define LWS_OUT_WIDTH 16
#define LWS_PRECISION 4
#define LWS_OUT_FREQ 32

/* default verbosity level: print nothing extra */
#define LWS_DEF_VERBOSITY_LEVEL -1


/* percentile setting */
#define LWS_SIGNIFICANT_BITS 3
#define LWS_MIN_VALUE 1
#define LWS_MAX_VALUE 3600000000LL

/* Simulation options */
#define LWS_THROUGHPUT -1
#define LWS_DELAY_VS_LOAD 0
#define LWS_DELAY_VS_BURST_SIZE 1

#define LWS_DEF_LOAD_TH 0.99
#define LWS_DEF_PORT_NUM 64
#define LWS_DEF_FRAMES 1000


/* maximum number of multiple values for the loads or burst sizes */
#define LWS_MAX_MULTI 64

// /* useful macros for warping common used functions */
//#define ALLOC(type, num)	\
//    ((type *) malloc(sizeof(type) * (num)))
//
//#define REALLOC(type, obj, num)	\
//    (obj) ? ((type *) realloc((char *) obj, sizeof(type) * (num))) : \
//    ((type *) malloc(sizeof(type) * (num)))
//
//#define FREE(obj)		\
//    ((obj) ? (free((char *) (obj)), (obj) = 0) : 0)



/* ---------------------------------------------------------------------------------- */
/*                             Objects                                                */
/* ---------------------------------------------------------------------------------- */
struct packet_t {
    int src;
    int dest;
    int time;
    packet_t(): src(0), dest(0), time(0) {}
    packet_t(int s, int d, int t): src(s), dest(d), time(t) {
        //std::cout << "create a packet" << std::endl;
    }
    // int index; /*! needed by queue proportional sampling to eliminate hash table */
};

typedef struct packet_t  packet_t;

struct burst_t {
    int on;
    int dest;
    burst_t(): on(0), dest(LWS_NOARRIVAL){}
    burst_t(int o, int d): on(o), dest(d) {}
};

typedef struct burst_t burst_t;

typedef std::vector<std::vector<std::queue<std::shared_ptr<packet_t> > > > queue_t;

static const std::string MODEL_NAMES[4] = {
        "uniform",
        "log-diagonal",
        "quasi-diagonal",
        "diagonal"
};

static const std::string INJECTION_NAMES[2] = {
        "Bernoulli",
        "on-off-burst"
};

static const std::string DEF_LOGGING_NAME = "../logging/default_logging.txt";


#ifndef LWS_VERSION
/**
 * This part of codes are for backward compatiblity
 */
typedef struct { /* lightweight switch parameters object */
    std::string name;   		/* name of the simulator */
    int type;              /* type of the simulation */

    int model_option; 		/* traffic model option (default = uniform) */
    std::string model_name; 		/* traffic model name */

    int inject_option; 		/* injection option (default = Bernoulli) */
    std::string inject_name; 		/* injection name */

    int N;				    /* number of ports */
    int MAX_M;  			/* number of macroframes */

    unsigned seed;          /* seed for traffic generator */

    int verbosity;         /* printing controller */

    std::string logging_name;    /* logging file name */

    double average_burst_length;  /* current average burst length for geometric burst */
    std::vector<double> abls; /* burst size to be simulated */
    int num_abls; /* number of average burst size to be simulated */
    double prob_on; /* probability for on to off */
    double prob_off; /* probability for off to on */

    double minimum_load; /* minimum load */
    double maximum_load; /* maximum load */


    double load_step; /* load step */

    std::vector<double> loads; /* traffic loads to be simulated */
    int num_loads; /* number of loads to be simulated */
    double load; /* current simulated normalized load */

    //double foo_bar[8]; /* reserved for future usage */
} lws_param_t;


typedef struct {/* lightweight switch status */
    std::vector<int> A; /* arrivals */
    /* int *D; */ /* departures */
    int cur_time; /* current time slot */

    // std::vector<std::shared_ptr<burst_t> > burst; /* burst status */
    queue_t B; /* Virtual output queue status */

    /* queue length for each virtual output queue,
     * it is used for scheduling algorithms that cannot
     * guarantee 100% throughput. Because if the scheduling
     * algorithm is not stable, the virtual output queue would
     * be with length infinite, therefore, we can not hold packets
     * in the queues. In that case, we will just store how many packets
     * are in each queues.
     * */
    std::vector<std::vector<int> > Q;
    std::vector<int> S; /* schedule */
} lws_status_t;

typedef struct {
    int max_delay;			/* max delay */
    double mean_delay;  		/* mean delay or queue length */
    double mean_delay_count;	/* used in mean delay calc */
    double sum_delay;           /* sum of delays from all VOQs */

    double T_arr;  			/* total arrival */
    double T_dep;  			/* total departure */

    std::ofstream logger;              /* logging file pointer */

    struct hdr_histogram *histogram; /* for percentile delay */

    double foo_bar[8];    /* reserved for future usage */

} lws_inst_t;

class LWS_SwitchCore {
public:
    cxxopts::Options options;
    lws_param_t params;
    lws_status_t status;
    lws_inst_t instruments;
    enum STATE {
        CREATED,
        PARSER_CONFIGED,
        PARSED,
        INITED
    };
    STATE s;
    int show_counter;

    LWS_SwitchCore(char *name): options(name, " - command line options"){
        s = CREATED;
        show_counter = 0;
    }
    void config(){
        try {
            options.add_options()
                    ("p,port", "port number", cxxopts::value<int>(), "N")
                    ("m,frames", "frame number", cxxopts::value<int>(), "N")
                    ("t,traffic", "traffic matrix model (u, l, q, d)", cxxopts::value<char>(), "C")
                    ("l,loads", "traffic loads", cxxopts::value<std::vector<double> >(), "LOAD")
                    ("v,verbosity", "verbosity level", cxxopts::value<int>(), "N")
                    ("b,burst", "burst size", cxxopts::value<std::vector<double> >(), "BUSRT_SIZE")
                    ("s,seed", "seed for traffic generator", cxxopts::value<unsigned>(), "SEED")
                    ("f,file", "log file", cxxopts::value<std::string>(), "FILE")
                    ("T,Throughput", "Measure throughput under which load",cxxopts::value<double>(), "LOAD")
                    ("n,name", "simulator name", cxxopts::value<std::string>(), "NAME")
                    ("h,help", "Print help");
        }catch (const cxxopts::OptionException& e){
            std::cerr << "Error while configuring parser: " << e.what() << std::endl;
            exit(1);
        }
        s = PARSER_CONFIGED;
    }
    void parsing(int argc, char *argv[]){
            //std::cout << "argc = " << argc << std::endl;
        int argc_bk = argc; /* pay attention */

        params.type = LWS_DELAY_VS_LOAD;
        params.inject_option = LWS_BERNOULLI_IID;
        params.model_option = LWS_ALL_MODELS;
        params.name = "Simulator";

        params.seed = std::chrono::system_clock::now().time_since_epoch().count(); /* default seed */
        try{
            options.parse(argc, argv);/* Note that, here parse will change the value of argc and argv */
            //std::cout << options.count("h") << std::endl;
            if (options.count("h") || argc_bk == 1) {std::cout << options.help() << std::endl; exit(0); }
            if (options.count("n")) params.name = options["n"].as<std::string>();
            if (options.count("p")) params.N = options["p"].as<int>();
            else params.N = LWS_DEF_PORT_NUM;
            if (options.count("m")) params.MAX_M = options["m"].as<int>();
            else params.MAX_M = LWS_DEF_FRAMES;
            params.MAX_M *= params.N * params.N;
            if (options.count("b")) {
                params.abls = options["b"].as<std::vector<double> >();
                params.type = LWS_DELAY_VS_BURST_SIZE;
            }
            if (options.count("l")) params.loads = options["l"].as<std::vector<double> >();
            else {
                if (params.type == LWS_BERNOULLI_IID)
                    params.loads = {0.99, 0.95, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1};
                else if (params.type == LWS_ON_OFF_BURST)
                {
                    throw cxxopts::OptionException("at least one traffic load value should be entered, when you use -b");
                }
            }
            if (options.count("s")) params.seed = options["s"].as<unsigned>();

            if (params.abls.size() == 0) params.inject_option = LWS_BERNOULLI_IID;
            else params.inject_option = LWS_ON_OFF_BURST;
            params.inject_name = INJECTION_NAMES[params.inject_option];
            if (options.count("v")) params.verbosity = options["v"].as<int>();
            else params.verbosity = 0;
            if (options.count("f")) params.logging_name = options["f"].as<std::string>();
            else params.logging_name = DEF_LOGGING_NAME;
            if (options.count("t")) {
                switch(options["t"].as<char>()){
                    case 'u':
                        params.model_option = LWS_UNIFORM;
                        break;
                    case 'l':
                        params.model_option = LWS_LOG_DIAGONAL;
                        break;
                    case 'q':
                        params.model_option = LWS_QUASI_DIAGONAL;
                        break;
                    case 'd':
                        params.model_option = LWS_DIAGONAL;
                        break;
                    default:
                        params.model_option = LWS_ALL_MODELS;
                }
            }
            if (params.model_option != LWS_ALL_MODELS) params.model_name = MODEL_NAMES[params.model_option];
            if (options.count("T")) {
                params.type = LWS_THROUGHPUT;
                params.loads.clear();
                params.loads.push_back(options["T"].as<double>());
            }
            //
        }catch (const cxxopts::OptionException &e){
            std::cerr << "while parsing parameters: " << e.what() << "\n\n" << std::endl;
            std::cout << options.help() << std::endl;
            exit(1);
        }
        s = PARSED;
    }
    void print_params() const {
        nlohmann::json j2 = {
                {"name", params.name},
                {"type", params.type},
                {"model_option", params.model_option},
                {"model_name", params.model_name},
                {"injection_option", params.inject_option},
                {"injection_name", params.inject_name},
                {"port_number", params.N},
                {"frame_number", params.MAX_M},
                {"logging_file", params.logging_name},
                {"loads", params.loads},
                {"abls", params.abls},
        };
        std::cout << j2.dump(4) << std::endl;
    }
    void print_status() const {
        if (s != INITED) {std::cerr << "You have not inited, please init first" << std::endl; return;}
        nlohmann::json j = {
                {"arrivals", status.A },
                {"schedule", status.S },
                {"VOQs", status.Q },
                {"current_time", status.cur_time}
        };
        std::cout << j.dump(4) << std::endl;
    }
    void print_instruments() const {
        if (s != INITED) {std::cerr << "You have not inited, please init first" << std::endl; return;}
        nlohmann::json j = {
                {"arrivals", instruments.T_arr},
                {"departures", instruments.T_dep},
                {"mean_delay", instruments.mean_delay},
                {"max_delay", instruments.max_delay},
                {"delay_counter", instruments.mean_delay_count},
                {"sum_delay", instruments.sum_delay}
        };
        std::cout << j.dump(4) << std::endl;
        hdr_percentiles_print(
                instruments.histogram,
                stdout,
                5,
                1.0,
                CLASSIC);
    }
    void print_all() const {
        print_params();

        print_status();
        print_instruments();
    }
    void init(int argc, char *argv[]) {
        if (s == CREATED) {config(); parsing(argc, argv);}
        else if (s == PARSER_CONFIGED) {parsing(argc, argv);}
        init();
    }
    void init() {
        if (s == CREATED) {
            std::cerr << "you have not configure your switch, please call configure first! " << std::endl;
            return;
        }
        status.A.resize(params.N);
        for (int i = 0;i < params.N;++ i) status.A[i] = LWS_NOARRIVAL; /* initilized as no arrivals */
        //std::cout << "A" << std::endl;
        /*
        if (params.inject_option == LWS_ON_OFF_BURST) {
            status.burst.resize(params.N);
            for (int i  = 0;i < params.N; ++ i) status.burst[i] = std::make_shared<burst_t>(0, LWS_NOARRIVAL);
        }
        */

        //std::cout << "b" << std::endl;
        status.cur_time = 0;
        status.Q.resize(params.N);
        //std::cout << "Q" << std::endl;
        for (int i = 0;i < params.N;++ i) {
            status.Q[i].resize(params.N);
            for (int k = 0;k < params.N;++ k) status.Q[i][k] = 0;
        }
        //std::cout << "Q" << std::endl;
        status.S.resize(params.N);
        for (int i = 0;i < params.N;++ i) status.S[i] = i; /* initilze as identical matching */
        //std::cout << "S" << std::endl;
        if (params.type != LWS_THROUGHPUT) { /* fixed */
            status.B.resize(params.N);
            for (int i = 0;i < params.N;++ i) status.B[i].resize(params.N);
        }
        //std::cout << "B" << std::endl;

        status.cur_time = 0;

        instruments.mean_delay_count = 0;
        instruments.mean_delay = 0;
        instruments.max_delay = 0;
        instruments.sum_delay = 0;
        instruments.T_arr = 0;
        instruments.T_dep = 0;
        instruments.logger.open(params.logging_name);

        hdr_init(
                LWS_MIN_VALUE,
                LWS_MAX_VALUE,
                LWS_SIGNIFICANT_BITS,
                &(instruments.histogram)
        );

        s = INITED;
    }
    void reset(){
        std::cout << "reset:: start ..." << std::endl;
        if (s != INITED) { std::cerr << "You have not init, please init first" << std::endl; return; }

        std::cout << "reset:: A ..." << std::endl;
        assert(params.N == status.A.size());
        for (int i = 0;i < params.N;++ i) status.A[i] = LWS_NOARRIVAL;

        /*
        if (params.inject_option == LWS_ON_OFF_BURST) {
            std::cout << "reset:: burst ..." << std::endl;
            assert(params.N == status.burst.size());
            for (int i  = 0;i < params.N; ++ i) status.burst[i] = std::make_shared<burst_t>(0, LWS_NOARRIVAL);
        }
        */

        std::cout << "reset:: Q ..." << std::endl;
        assert(params.N == status.Q.size());
        for (int i = 0;i < params.N;++ i) {
            assert(params.N == status.Q[i].size());
            for (int k = 0;k < params.N;++ k) status.Q[i][k] = 0;
        }

        if (params.type != LWS_THROUGHPUT) { /* fixed */
            std::cout << "reset:: B ..." << std::endl;
            assert(params.N == status.B.size());
            for (int i = 0;i < params.N;++ i) {
                assert(params.N == status.B[i].size());
                for (int k = 0;k < params.N;++ k) {
                    while (!status.B[i][k].empty()) status.B[i][k].pop();
                }
            }
        }

        std::cout << "reset:: S ..." << std::endl;
        assert(status.S.size() == params.N);
        for (int i = 0;i < params.N;++ i) status.S[i] = i;

        std::cout << "reset:: instruments ..." << std::endl;

        instruments.mean_delay_count = 0;
        instruments.mean_delay = 0;
        instruments.max_delay = 0;
        instruments.sum_delay = 0;
        instruments.T_arr = 0;
        instruments.T_dep = 0;

        std::cout << "reset histogram ..." << std::endl;

        hdr_reset(instruments.histogram);
    }
    void destroy() {
        if (s != INITED) return;
        status.A.clear();
        // status.burst.clear();
        for (int i = 0;i < params.N;++ i) {
            status.Q[i].clear();
            if (params.type != LWS_THROUGHPUT)
            {
                for (int k = 0;k < params.N;++ k)
                    while (!status.B[i][k].empty()) status.B[i][k].pop();
                status.B[i].clear();
            }
        }
        status.Q.clear();
        status.B.clear();
        status.S.clear();

        instruments.logger.close();

        if (instruments.histogram != NULL) free(instruments.histogram);


        s = CREATED;
    }
    void show(std::ostream& out=std::cout, bool reset_counter=false){

        if (show_counter == 0) show_header(out);


        out       << std::setw(6)
                  << std::left
                  << std::fixed
                  << std::setprecision(2)
                  << params.load
                  << std::setw(13)
                  << std::left
                  << params.model_option;

        if (params.inject_option == LWS_ON_OFF_BURST)
             out      << std::setw(11)
                      << std::left
                      << std::fixed
                      << std::setprecision(2)
                      << params.average_burst_length;

        out           << std::setw(12)
                      << std::left
                      << std::fixed
                      << std::setprecision(LWS_PRECISION)
                      << ((instruments.T_arr == 0)? 0:(instruments.T_dep / instruments.T_arr))
                      << std::setw(16)
                      << std::left
                      << ((instruments.mean_delay_count == 0)? -1: (instruments.sum_delay / instruments.mean_delay_count));

        out           << std::setw(12)
                      << std::left
                      << instruments.max_delay;

        out           << std::setw(16)
                      << std::left
                      << std::fixed
                      << std::setprecision(LWS_PRECISION)
                      << hdr_mean(instruments.histogram) - 1;

        out           << std::setw(14)
                      << std::left
                      << hdr_max(instruments.histogram) - 1;

        out           << std::setw(15)
                      << std::left
                      << std::fixed
                      << std::setprecision(LWS_PRECISION)
                      << hdr_stddev(instruments.histogram);
        out           << std::setw(12)
                      << std::left
                      << hdr_value_at_percentile(instruments.histogram,90) - 1
                      << std::setw(12)
                      << std::left
                      << hdr_value_at_percentile(instruments.histogram,95) - 1
                      << std::setw(12)
                      << std::left
                      << hdr_value_at_percentile(instruments.histogram,99) - 1
                      << std::endl;

        if (reset_counter) show_counter = 0;
        else ++ show_counter;

    }
    void show_header(std::ostream& out=std::cout, bool update_counter=false){


            out << std::setw(6)
                << std::left
                << "#Load"
                << std::setw(13)
                << std::left
                << "Traffic-Mode";
        if (params.inject_option == LWS_ON_OFF_BURST)
            out     << std::setw(11)
                    << std::left
                    << "Burst-Size";

        out     << std::setw(12)
                << std::left
                << "Throughput"
                << std::setw(16)
                << std::left
                << "Mean-Delay"
                << std::setw(12)
                << std::left
                << "Max-Delay"
                << std::setw(16)
                << std::left
                << "Mean-Delay-HDR"
                << std::setw(14)
                << std::left
                << "Max-Delay-HDR"
                << std::setw(15)
                << std::left
                << "Delay-STD-HDR"
                << std::setw(12)
                << std::left
                << "P90-Delay"
                << std::setw(12)
                << std::left
                << "P95-Delay"
                << std::setw(12)
                << std::left
                << "P99-Delay"
                << std::endl;

        if (update_counter) ++ show_counter;
    }
};

#else


namespace nlohmann{
    /** merge two json object and store the results in the first one */
    void merge(json& j1, json& j2){
        for (json::iterator it=j2.begin();it != j2.end();++ it)
            j1.push_back(json::object_t::value_type(it.key(),it.value()));
    }
}

/* parameter base class */
class paramBase {
public:
    cxxopts::Options options; /* command line parser */
    std::string name;           /* name of the simulator */
    int type;              /* type of the simulation */

    int model_option;       /* traffic model option (default = uniform) */
    std::string model_name;         /* traffic model name */

    int inject_option;      /* injection option (default = Bernoulli) */
    std::string inject_name;        /* injection name */

    int N;                  /* number of ports */
    int MAX_M;              /* number of macroframes */

    unsigned seed;          /* seed for traffic generator */

    int verbosity;         /* printing controller */

    double average_burst_length;  /* current average burst length for geometric burst */
    std::vector<double> abls; /* burst size to be simulated */

    double prob_on; /* probability for on to off */
    double prob_off; /* probability for off to on */


    std::vector<double> loads; /* traffic loads to be simulated */

    double load; /* current simulated normalized load */

    paramBase(): options("Simulator", " - command line options"), name(""), type(-1), model_option(-1),
    model_name(""), inject_option(-1), inject_name(""), N(-1), MAX_M(-1), seed(0), verbosity(-1), average_burst_length(-1),
    abls(), prob_on(-1), prob_off(-1), loads(), load(-1) {
    }
    paramBase(char *name): options(name, " - command line options"), name(""), type(-1), model_option(-1),
                           model_name(""), inject_option(-1), inject_name(""), N(-1), MAX_M(-1), seed(0), verbosity(-1), average_burst_length(-1),
                           abls(), prob_on(-1), prob_off(-1), loads(), load(-1) {
    }
    explicit  paramBase(int argc, char *argv[]): options(argv[0], " - command line options"), name(""), type(-1), model_option(-1),
    model_name(""), inject_option(-1), inject_name(""), N(-1), MAX_M(-1), seed(0), verbosity(-1), average_burst_length(-1),
    abls(), prob_on(-1), prob_off(-1), loads(), load(-1) {
        parser_config();
        parser_parse(argc, argv);
    }

    virtual nlohmann::json to_json() const {
        nlohmann::json j = {
                {"name", name},
                {"type", type},
                {"model_option", model_option},
                {"model_name", model_name},
                {"injection_option", inject_option},
                {"injection_name", inject_name},
                {"port_number", N},
                {"frame_number", MAX_M},
                {"verbosity_level", verbosity},
                {"seed_for_traffic_generator", seed},
                {"average_burst_length_array", abls},
                {"probability_on", prob_on},
                {"probability_off", prob_off},
                {"load_array", loads},
                {"load", load},
                {"average_burst_length", average_burst_length}
                            };
        return j;
    }
    /**
     * configure command line parser
     */
    virtual void parser_config(){
        try{
            options.add_options()
                    ("p,port", "port number", cxxopts::value<int>(), "N")
                    ("m,frames", "frame number", cxxopts::value<int>(), "N")
                    ("t,traffic", "traffic matrix model (u, l, q, d)", cxxopts::value<char>(), "C")
                    ("l,loads", "traffic loads", cxxopts::value<std::vector<double> >(), "LOAD")
                    ("v,verbosity", "verbosity level", cxxopts::value<int>(), "N")
                    ("b,burst", "burst size", cxxopts::value<std::vector<double> >(), "BUSRT_SIZE")
                    ("s,seed", "seed for traffic generator", cxxopts::value<unsigned>(), "SEED")
                    ("T,Throughput", "Measure throughput under which load",cxxopts::value<double>(), "LOAD")
                    ("n,name", "simulator name", cxxopts::value<std::string>(), "NAME")
                    ("h,help", "Print help");
        }catch (const cxxopts::OptionException& e){
            std::cerr << "Error while configuring parser: " << e.what() << std::endl;
            exit(1);
        }
    }
    /**
     * parse command line options
     * @param argc  number of options
     * @param argv  options
     */
    virtual void parser_parse(int argc, char* argv[]) {
        int argc_bk = argc; /* pay attention */

        /*
        * TODO: move these initialization to the constructor
        */

        type = LWS_DELAY_VS_LOAD;
        inject_option = LWS_BERNOULLI_IID;
        model_option = LWS_ALL_MODELS;
        name = "Simulator";

        seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()); /* default seed */

        try{
            options.parse(argc, argv);/* Note that, here parse will change the value of argc and argv */

            if (options.count("h") || argc_bk == 1) {std::cout << options.help() << std::endl; exit(0); }
            if (options.count("n")) name = options["n"].as<std::string>();
            if (options.count("p")) N = options["p"].as<int>();
            else N = LWS_DEF_PORT_NUM;
            if (options.count("m")) MAX_M = options["m"].as<int>();
            else MAX_M = LWS_DEF_FRAMES;
            MAX_M *= (N * N);
            if (options.count("b")) {
                abls = options["b"].as<std::vector<double> >();
                type = LWS_DELAY_VS_BURST_SIZE;
            }
            if (options.count("l")) loads = options["l"].as<std::vector<double> >();
            else {
                if (type == LWS_BERNOULLI_IID)
                    loads = {0.99, 0.95, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1};
                else if (type == LWS_ON_OFF_BURST)
                {
                    throw cxxopts::OptionException("At least one traffic load value should be entered, when you use -b");
                }
            }
            if (options.count("s")) seed = options["s"].as<unsigned>();

            if (abls.size() == 0) inject_option = LWS_BERNOULLI_IID;
            else inject_option = LWS_ON_OFF_BURST;
            inject_name = INJECTION_NAMES[inject_option];
            if (options.count("v")) verbosity = options["v"].as<int>();
            else verbosity = LWS_DEF_VERBOSITY_LEVEL;
            if (options.count("t")) {
                // TODO: allow user to input any combinations of the four
                // TODO: allow user to use customized traffic matrix
                switch(options["t"].as<char>()){
                    case 'u':
                        model_option = LWS_UNIFORM;
                        break;
                    case 'l':
                        model_option = LWS_LOG_DIAGONAL;
                        break;
                    case 'q':
                        model_option = LWS_QUASI_DIAGONAL;
                        break;
                    case 'd':
                        model_option = LWS_DIAGONAL;
                        break;
                    case 'a':
                        model_option = LWS_ALL_MODELS;
                        break;
                    default:
                        throw cxxopts::OptionException("Unknown value for option -t, which only accept a|u|l|q|d, ");
                }
            }
            if (model_option != LWS_ALL_MODELS) model_name = MODEL_NAMES[model_option];
            if (options.count("T")) {
                type = LWS_THROUGHPUT;
                loads.clear();
                loads.push_back(options["T"].as<double>());
            }

        }catch (const cxxopts::OptionException &e){
            std::cerr << "While parsing parameters: " << e.what() << "\n\n" << std::endl;
            std::cout << options.help() << std::endl;
            exit(1);
        }
    }
    virtual void clear()
    {// reserved for future usage

    }
};

/*
 * create to_string for paramBase
 */
namespace std{
    string to_string(::paramBase const& params){
        nlohmann::json j2 = params.to_json();
        return j2.dump(4);
    }

}


class paramSerenade: public paramBase {
public:
    std::string Ouroboros_filename; // file recording which numbers are O which are not
    std::string Ouroboros_stats_filename; // file to record statistic results
    int Q_bound; // queue length threshold for trigger the transition from O-Serenade to C-Serenade
    double frequency;

    paramSerenade(): paramBase(), Ouroboros_filename(""), Ouroboros_stats_filename(""),
                     Q_bound(std::numeric_limits<int>::max()), frequency(std::numeric_limits<double>::max()) {
    }
    paramSerenade(char *name): paramBase(name), Ouroboros_filename(""), Ouroboros_stats_filename(""),
                               Q_bound(std::numeric_limits<int>::max()), frequency(std::numeric_limits<double>::max()) {
    }
    explicit paramSerenade(int argc, char *argv[]): paramBase(argv[0]), Ouroboros_filename(""),
                                                    Ouroboros_stats_filename(""),
                                                    Q_bound(std::numeric_limits<int>::max()),
                                                    frequency(std::numeric_limits<double>::max()) {
        parser_config();
        parser_parse(argc, argv);

        Ouroboros_filename = fmt::format("../data/Ouroboros/n={0}.txt", N);
        Ouroboros_stats_filename = fmt::format("../results/{name}-{N}-{FRAME}-{RANDOM}.dat",
                                               fmt::arg("name", name),
                                               fmt::arg("N", N),
                                               fmt::arg("FRAME", MAX_M),
                                               fmt::arg("RANDOM", seed));
    }

    virtual nlohmann::json to_json() const {
        nlohmann::json jp = paramBase::to_json();
        nlohmann::json jn = {
                {"Ouroboros_filename", Ouroboros_filename},
                {"Ouroboros_stats_filename", Ouroboros_stats_filename},
                {"Queue_bound", Q_bound},
                {"E_Frequency", frequency}
        };
        nlohmann::merge(jp, jn);
        return jp;
    }

    virtual void parser_config() {
        paramBase::parser_config();
        try {
            options.add_options()
                    ("O,Ouroboros", "Ouroboros file", cxxopts::value<std::string>(), "FILENAME")
                    ("I,Instrument", "Instrument file for Ouroboros related statistical analysis", cxxopts::value<std::string>(), "FILENAME")
                    ("B,Bound", "Queue length bound for O-Serenade", cxxopts::value<int>(), "N")
                    ("F,Frequency", "Frequency to use E-Serenade", cxxopts::value<double>(), "N");
        }catch (const cxxopts::OptionException& e){
            std::cerr << "Error while configuring parser: " << e.what() << std::endl;
            exit(1);
        }
    }

    virtual void parser_parse(int argc, char* argv[]) {
        paramBase::parser_parse(argc, argv);

//        if (argc <= 1) return; // no remaining
        unsigned long r = static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count()); /* default seed */

        try{
            if (options.count("O")) Ouroboros_filename = options["O"].as<std::string>();
            if (options.count("S")) {
                Ouroboros_stats_filename = options["S"].as<std::string>();
                Ouroboros_stats_filename += (std::to_string(r) + ".dat");
            }
            if (options.count("B")) Q_bound = options["B"].as<int>();
            if (options.count("F")) frequency = options["F"].as<double>();

        }catch (const cxxopts::OptionException &e){
            std::cerr << "While parsing parameters: " << e.what() << "\n\n" << std::endl;
            std::cout << options.help() << std::endl;
            exit(1);
        }
    }

    virtual void clear()
    {// reserved for future usage

    }
};

/**
    base class for instrument
*/
class instrumentBase {
public:
    int max_delay;			/* max delay */
    double mean_delay;  		/* mean delay or queue length */
    double mean_delay_count;	/* used in mean delay calc */
    double sum_delay;           /* sum of delays from all VOQs */

    double T_arr;  			/* total arrival */
    double T_dep;  			/* total departure */

    struct hdr_histogram *histogram; /* for percentile delay */

    std::string fmt_template;
    std::string fmt_template_header;

    instrumentBase(): max_delay(0), mean_delay(0), mean_delay_count(0),
                      sum_delay(0), T_arr(0), T_dep(0), histogram(NULL),
                      fmt_template("{throughput:<16.4f}{mean_delay:<16.4f}{max_delay:<12}{ninety:<12}{ninety_five:<12}{ninety_nine:<12}"),
                      fmt_template_header("{throughput:<16}{mean_delay:<16}{max_delay:<12}{ninety:<12}{ninety_five:<12}{ninety_nine:<12}")
                      {
        hdr_init(
                LWS_MIN_VALUE,
                LWS_MAX_VALUE,
                LWS_SIGNIFICANT_BITS,
                &(histogram)
        );
    }

    virtual void reset() {
        mean_delay_count = 0;
        mean_delay = 0;
        max_delay = 0;
        sum_delay = 0;
        T_arr = 0;
        T_dep = 0;

        hdr_reset(histogram);
    }

    virtual std::string to_string(bool header=false) const {
        if (!header)
            return fmt::format(fmt_template_header,
            fmt::arg("throughput",((T_arr == 0)? 0:(T_dep / T_arr))),
            fmt::arg("mean_delay", ((mean_delay_count == 0)? -1: (sum_delay / mean_delay_count))),
            fmt::arg("max_delay", max_delay),
            fmt::arg("ninety", hdr_value_at_percentile(histogram,90) - 1),
            fmt::arg("ninety_five", hdr_value_at_percentile(histogram,95) - 1),
            fmt::arg("ninety_nine", hdr_value_at_percentile(histogram,99) - 1));
        else
            return fmt::format(fmt_template_header,
                                fmt::arg("throughput","Throughput"),
                                fmt::arg("mean_delay", "Mean-Delay"),
                                fmt::arg("max_delay", "Max-Delay"),
                                fmt::arg("ninety", "P90-Delay"),
                                fmt::arg("ninety_five", "P95-Delay"),
                                fmt::arg("ninety_nine", "P99-Delay"));

    }
    virtual void clear(){
        if (histogram != NULL) {
            free(histogram);
            histogram = NULL;
        }
    }

    virtual ~instrumentBase(){
//        std::cout << "Base deconstructor is called " << std::endl;
        if (histogram != NULL) {
            free(histogram);
            histogram = NULL;
        }
    }
};

// create to_string for instrumentBase
namespace std {
    string to_string(::instrumentBase const& instruments) {
            return instruments.to_string();
    }
}


class instrumentSerenade: public instrumentBase {
public:

    double C_counter_T; /* number of cycles */
    double C_counter_O; /* number of ouroboros cycles */
    double C_counter_W; /* number of non-ouroboros cycles which make wrong decisions */

    double C_counter_BC; /* number of cycle back to C-Serenade */

    double W_counter_T; /* weight of cycles */
    double W_counter_O; /* weight of ouroboros cycles */
    double W_counter_W; /* weight of non-ouroboros cycles which make wrong decision */


    double V_counter_O_woE; /* vertices belong to ouroboros */
    double V_counter_T_woE; /* total number of vertices */
    double V_counter_nonO_W_woE; /* vertices belong to non-ouroboros cycles and make different decision as SERENA */

    double N_counter_AO_woE; /* number of time slots, in which all cycles are ouroboros */
    double N_counter_T_woE; /* total time slots */
//    double N_counter_BC; /* number of time slots, back to C-SERENADE */
//    double N_counter_AO_BC; /* number of time slot, back to C-SERENADE and all ouroboros */

    double N_counter_nonO_E; /* number of time slots running E-SERENADE, in which non-ouroboros cycles appear */
    double N_counter_TE; /* number of total time slots running E-SERENADE */
    double BSIT_counter_E; /* number of binary search iterations involving message passing */


    double C_counter_nonO_woE; /* number of non-ouroboros cycles for O-SERENADE */
    double C_counter_nonO_E; /* number of non-ouroboros cycles for E-SERENADE */

    // distribution counter
    std::vector<double> C_size_hist; /* cycle size histogram */

    std::vector<double> C_size_hist_for_E; /* cycle size histogram for E-SERENADE */

    // total variables
    int tn;

    explicit instrumentSerenade(int pn): instrumentBase(), C_counter_T(0), C_counter_O(0), C_counter_W(0), C_counter_BC(0),
                                         W_counter_T(0), W_counter_O(0), W_counter_W(0),
                                         V_counter_O_woE(0), V_counter_T_woE(0), V_counter_nonO_W_woE(0),
                                         N_counter_AO_woE(0), N_counter_T_woE(0), // N_counter_BC(0), N_counter_AO_BC(0),
                                         N_counter_nonO_E(0), N_counter_TE(0), BSIT_counter_E(0),
                                         C_counter_nonO_woE(0), C_counter_nonO_E(0),
                                         C_size_hist(pn, 0), C_size_hist_for_E(pn, 0), tn(2 * pn + 17){
                                         }
    virtual void reset(){
        instrumentBase::reset();

        C_counter_T = 0;
        C_counter_O = 0;
        C_counter_W = 0;

        C_counter_BC = 0;

        W_counter_T = 0;
        W_counter_O = 0;
        W_counter_W = 0;

        V_counter_O_woE = 0;
        V_counter_T_woE = 0;
        V_counter_nonO_W_woE = 0;

        N_counter_AO_woE = 0;
        N_counter_T_woE = 0;
//        N_counter_BC = 0;
//        N_counter_AO_BC = 0;


        N_counter_nonO_E = 0;
        N_counter_TE = 0;
        BSIT_counter_E = 0;

        C_counter_nonO_woE = 0;
        C_counter_nonO_E = 0;

        std::fill(C_size_hist.begin(), C_size_hist.end(), 0);

        std::fill(C_size_hist_for_E.begin(), C_size_hist_for_E.end(), 0);

    }

    virtual std::string to_string(bool header=false) const {
        std::string s;
        if (header){
            s = fmt::format("{0:<14}{1:<14}{2:<14}{3:<14}{4:<14}{5:<14}{6:<14}{7:<14}{8:<14}{9:<14}{10:<14}{11:<14}{12:<14}{13:<14}{14:<14}{15:<14}{16:<14}",
                                        "cycles", "cycles-ourob", "cycles-wrong", "cycles-bc",
                                        "cyc-weights", "ocyc-weights", "wcyc-weights",
                                        "overtices-woE", "avertices-woE", "wrong-nonvertices-woE",
                                        "ts-ao-woE", "ts-all-woE", //"ts-bc", "ts-ao-bc",
                                        "ts-nonouro-E", "ts-all-E", "bsit-mp-E",
                                        "nonouro-cyc-woE", "nonouro-cyc-W");

            for (int i = 0;i < C_size_hist.size();++ i)
                s += fmt::format("{:<14}", "CS-" + std::to_string(i + 1));

            for (int i = 0;i < C_size_hist_for_E.size();++ i)
                s += fmt::format("{:<14}", "CSE-" + std::to_string(i + 1));
        }
        else
        {
            s = fmt::format("{0:<14}{1:<14}{2:<14}{3:<14}{4:<14}{5:<14}{6:<14}{7:<14}{8:<14}{9:<14}{10:<14}{11:<14}{12:<14}{13:<14}{14:<14}{15:<14}{16:<14}",
                                        C_counter_T, C_counter_O, C_counter_W, C_counter_BC,
                                        W_counter_T, W_counter_O, W_counter_W,
                                        V_counter_O_woE, V_counter_T_woE, V_counter_nonO_W_woE,
                                        N_counter_AO_woE, N_counter_T_woE, // N_counter_BC, N_counter_AO_BC,
                                        N_counter_nonO_E, N_counter_TE, BSIT_counter_E,
                                        C_counter_nonO_woE, C_counter_nonO_E
                                        );
            for (auto x: C_size_hist)
                s += fmt::format("{:<14.0f}", x);

            for (auto x: C_size_hist_for_E)
                s += fmt::format("{:<14.0f}", x);
        }


        return instrumentBase::to_string(header) + s;
    }
    virtual void clear()
    {
        instrumentBase::clear();
        C_size_hist.clear();
        C_size_hist_for_E.clear();
    }
    virtual ~instrumentSerenade(){
//        instrumentBase::~instrumentBase(); // pay attention, parent's deconstructor will be called automatically
        C_size_hist.clear();
        C_size_hist_for_E.clear();
    }
};

namespace std {
    string to_string(::instrumentSerenade const& instruments) {
        return instruments.to_string();
    }
}


/**
    base class for status
*/
class statusBase {
public:
    std::vector<int> A; /* arrivals */
    /* int *D; */ /* departures */
    int cur_time; /* current time slot */

    // std::vector<std::shared_ptr<burst_t> > burst; /* burst status */
    queue_t B; /* Virtual output queue status */

    /* queue length for each virtual output queue,
     * it is used for scheduling algorithms that cannot
     * guarantee 100% throughput. Because if the scheduling
     * algorithm is not stable, the virtual output queue would
     * be with length infinite, therefore, we can not hold packets
     * in the queues. In that case, we will just store how many packets
     * are in each queues.
     * */
    std::vector<std::vector<int> > Q;
    std::vector<int> S; /* schedule */

    int port_num;

    explicit statusBase(int pn, bool throughput_only=false): A(pn, LWS_NOARRIVAL), cur_time(0),
                        B(), Q(pn, std::vector<int>(pn, 0)), S(pn, 0), port_num(pn) {
        for (int i = 0;i < pn;++ i) S[i] = i; /* initilze as identical matching */
        if (!throughput_only) {
            B.resize(pn);
            for (int i = 0;i < pn;++ i) B[i].resize(pn);
        }
    }

    virtual void reset() {

        cur_time = 0;

        std::fill(A.begin(), A.end(), LWS_NOARRIVAL);

        for (int i = 0;i < port_num;++ i) {
            for (int k = 0;k < port_num;++ k) Q[i][k] = 0;
        }

        if (!B.empty()) {
            for (int i = 0;i < port_num;++ i) {
                for (int k = 0;k < port_num;++ k) {
                    while (!B[i][k].empty()) B[i][k].pop(); /* dequeue all packets */
                }
            }
        }

        for (int i = 0;i < port_num;++ i) S[i] = i; /* reset to identical matching */
    }

    virtual void clear()
    {
        A.clear();

        for (int i = 0;i < port_num;++ i) {
            Q[i].clear();
            if (!B.empty())
            {
                for (int k = 0;k < port_num;++ k)
                    while (!B[i][k].empty()) B[i][k].pop();
                B[i].clear();
            }
        }
        Q.clear();
        B.clear();
        S.clear();
    }
    virtual nlohmann::json to_json() const {
        nlohmann::json j = {
                {"current-timeslot", cur_time},
                {"arrivals", A},
                {"queue-lengths", Q},
                {"schedule", S}
        };
        return j;
    }
    virtual ~statusBase(){
        A.clear();

        for (int i = 0;i < port_num;++ i) {
            Q[i].clear();

            if (!B.empty()){
                for (int k = 0;k < port_num;++ k)
                    while (!B[i][k].empty()) B[i][k].pop();
                B[i].clear();
            }

        }
        Q.clear();
        B.clear();
        S.clear();
    }
};


namespace std{
    string to_string(::statusBase const& status){
        nlohmann::json j = status.to_json();
        return j.dump(4);
    }
}
#if defined(Serenade)

typedef paramSerenade lws_param_t ;
typedef statusBase lws_status_t;
typedef instrumentSerenade lws_inst_t;

#else

typedef paramBase lws_param_t ;
typedef statusBase lws_status_t;
typedef instrumentBase lws_inst_t;

#endif

/**
    new version for LWS_SwitchCore
        1) change its members from simple struct to classes
        2) remove state management
        3) use fmt to manage output format
*/

class LWS_SwitchCore {
public:
    lws_param_t params;
    lws_status_t status;
    lws_inst_t instruments;

    int show_counter;

    LWS_SwitchCore(int argc, char* argv[]): params(argc, argv), status(params.N, params.type == LWS_THROUGHPUT), instruments(params.N), show_counter(0){

    }

    void print_params() const {
        std::cout << std::to_string(params) << std::endl;
    }

    void print_status() const {
        std::cout << std::to_string(status) << std::endl;
    }

    void print_instruments() const {
        std::cout << std::to_string(instruments) << std::endl;
    }

    void print_all() const {
        print_params();
        print_status();
        print_instruments();
    }

    void reset(){
        show_counter = 0;
        status.reset();
        instruments.reset();
    }

    void destroy() {
        params.clear();
        status.clear();
        instruments.clear();
    }
    // CHANGES
    //     1. format of load (d --> f)
    //     2. format of burst length (d --> f)
    void show(std::ostream& out=std::cout, bool reset_counter=false){

        if (show_counter == 0) show_header(out);

        nlohmann::json j = params.to_json();
        // for debug
//        std::cout << j.dump(4) << std::endl;
//        std::cout << j["load"] << std::endl;
        // pay attention on fmt::arg
        std::string s = fmt::format("{load:<6.3f}{traffic_matrix:<13}",
                                   fmt::arg("load", (double)j["load"]),
                                   fmt::arg("traffic_matrix", j["model_option"]));

        if (params.inject_option == LWS_ON_OFF_BURST) s += fmt::format("{:<11.0f}", (double)j["average_burst_length"]);


        s += instruments.to_string();

        out << s << std::endl;

        if (reset_counter) show_counter = 0;
        else ++ show_counter;

    }

    void show_header(std::ostream& out=std::cout, bool update_counter=false){

        std::string s = fmt::format("{0:<6}{1:<13}", "#Load", "Traffic-Mode");

        if (params.inject_option == LWS_ON_OFF_BURST) s += fmt::format("{:<11}", "Burst-Size");

        out << (s + instruments.to_string(true))
            << std::endl;

        if (update_counter) ++ show_counter;
    }

    virtual ~LWS_SwitchCore(){
        // pay attention their deconstructors will be called automatically
        // params.clear();
        // status.clear();
        // instruments.clear();
    }
};
#endif


class TrafficGenerator{
public:
    int N;
    double load;
   // RV rv;
    int type;

    TrafficGenerator(int port_num, double tload, int ttype): N(port_num), load(tload), type(ttype) {

    }

    int traffic_entry_(int i, RV& rv)
    {
        int j, r = 0;
        double c, range, h;

        switch (type) {
            case LWS_UNIFORM:
            {/* uniform traffic */
                //std::cout << "i = " << i << ", uniform" << std::endl;
                r = rv.random_int(N);
                break;
            }
            case LWS_DIAGONAL:
            { /* diagonal */
                //std::cout << "diagonal" << std::endl;
                c = (double) rv.random_int(N);
                range = (((double) N) * 2.0) / 3.0;
                if (c < range) {
                    r = i;
                } else {
                    r = (i + 1) % N;
                }
                break;
            }
            case LWS_LOG_DIAGONAL:
            {/* log diagonal traffic */
                //std::cout << "log diagonal" << std::endl;
                c = (double) rv.random_int(N);
                if (c < (0.5 * ((double) N))) {
                    r = i;
                } else if (c < (0.75 * ((double) N))) {
                    r = (i + 1) % N;
                } else if (c < (0.875 * ((double) N))) {
                    r = (i + 2) % N;
                } else if (c < (0.9375 * ((double) N))) {
                    r = (i + 3) % N;
                } else if (c < (0.96875 * ((double) N))) {
                    r = (i + 4) % N;
                } else {
                    r = (i + 5) % N;
                }
                break;
            }
            case LWS_QUASI_DIAGONAL:
            {/* quasi diagonal traffic */
                //std::cout << "quasi diagonal" << std::endl;
                c = (double) rv.random_int(N);
                if (c < (0.5 * ((double) N))) {
                    r = i;
                } else {
                    h = 0.5;
                    for (j = 1; j < N; j++) {
                        h = h + (1.0 / (((double) (N - 1)) * 2.0));
                        if (c < (h * ((double) N))) {
                            r = (i + j) % N;
                            return r;
                        }
                    }
                    r = (i + (N - 1)) % N;
                }
                break;
            }
            default: {
                std::cerr << "Unsupported traffic matrix type!" << std::endl;
            }
        }
        return r;
    }

    virtual void run(std::vector<int>& A, RV& rv) {
        // std::cout << "Base Traffic Generator " << std::endl;
        // std::cout << "|A| = " << A.size() << ", N = " << N << ", load = " << load << std::endl;

        int i;
        for (i = 0;i < N;++ i) {
            if (rv.random_01() < load) {
                A[i] = traffic_entry_(i, rv);
            }
        }
    }
    virtual ~TrafficGenerator(){

    }



};

class BurstTrafficGenerator: public TrafficGenerator{
public:
    typedef struct burst_t {
        int on;
        int dest;
        burst_t(): on(0), dest(LWS_NOARRIVAL){}
        burst_t(int o, int d): on(o), dest(0){}
    } burst_t;
    std::vector<std::shared_ptr<burst_t> > burst;
    double prob_on;
    double prob_off;
    BurstTrafficGenerator(int port_num, double load, int ttype, double burst_size):\
        TrafficGenerator(port_num, load, ttype) {
        prob_on = 1.0 / (1.0 + burst_size);
        double tmp = load / (1.0 - load); /* on2off ratio */
        prob_off = tmp / (tmp + burst_size);
        burst.resize(N);
        for (int i = 0;i < N;++ i) { burst[i] = std::make_shared<burst_t>(0, LWS_NOARRIVAL); }
    }

    virtual void run(std::vector<int>& A, RV& rv) {
        // std::cout << "Derived Traffic Generator" << std::endl;
        int i, sbk;

        for (i = 0;i < N;++ i) {
            do
            {
                sbk = burst[i]->on;
                if (burst[i]->on) {/* previous circle on */
                    if (rv.random_01() < prob_on) /* transfer to off */
                        burst[i]->on = 0;
                } else { /* previous circle off */
                    if (rv.random_01() < prob_off) {/* transfer to on */
                        burst[i]->dest = traffic_entry_(i, rv);
                        burst[i]->on =  1;
                    }
                }
            } while (burst[i]->on != sbk);

            if (burst[i]->on) {/* if on, then generate traffic */
                A[i] = burst[i]->dest;
            }
        }
    }

    virtual ~BurstTrafficGenerator()
    {

    }
};

class SwitchEvent {
public:
    /**\brief Package arrival event
     *
     * @param params
     * @param status
     * @param instruments
     * @param tgen
     */
    static void packet_arrival(lws_param_t &params, lws_status_t &status, lws_inst_t &instruments, TrafficGenerator *tgen, RV& rv) {
        int i, k;

        /* init A */
        for (i = 0;i < params.N;++ i) status.A[i] = LWS_NOARRIVAL;

        /* generate traffic */
        tgen->run(status.A, rv);

        /* arrival */
        for (i = 0;i < params.N;++ i){
            k = status.A[i];

            //std::cout << "(" << i << ", " << k << ")" << std::endl;
            if (k != LWS_NOARRIVAL){
                assert(k >= 0 && k < params.N);
                /* instruments update arrivals */
                ++ instruments.T_arr;
                if (params.type != LWS_THROUGHPUT) status.B[i][k].push(std::make_shared<packet_t>(i, k, status.cur_time));

                ++ status.Q[i][k];
            }
        }

        //std::cout << "packet arrival " << std::endl;
    }
    /**\brief Package departure
     *
     * @param params
     * @param status
     * @param instruments
     */
    static void packet_departure(lws_param_t &params, lws_status_t &status, lws_inst_t &instruments){
        int i, k, delay;

        for (i = 0;i < params.N;++ i){
            k = status.S[i];
            if (k >= 0 && k < params.N){/* valid schedule */
                if (status.Q[i][k] > 0){/* cells available */

                    if (params.type != LWS_THROUGHPUT){

                        std::shared_ptr<packet_t> pkt;
                        pkt = status.B[i][k].front(); /* get info of head of line packet go */
                        assert(i == pkt->src && k == pkt->dest);
                        delay = status.cur_time - pkt->time;
                        status.B[i][k].pop(); /* let go hol packet */
                        if (delay > instruments.max_delay) instruments.max_delay = delay;
                        instruments.sum_delay += (double) delay;
                        instruments.mean_delay_count += 1.0;

                        hdr_record_value(
                                instruments.histogram,
                                delay + 1); /* change delay to delay + 1, since 0 is not supported */
                    }

                    -- status.Q[i][k]; /* pay attention */


                    /* instrument update_delays, and departure */
                    ++ instruments.T_dep;

                }
            }
        }

    }
};



class UT{
public:
    static inline void identical_schedule(std::vector<int>& S){
        for (int i = 0;i < S.size();++ i) S[i] = i;
//        nlohmann::json j = {
//                {"schedule", S}
//        };
//        std::cout << j.dump(4) << std::endl;
    }
    static inline void rpermute(std::vector<int>& S, RV& rv) {

        identical_schedule(S);
        for (int i = S.size() - 1;i >= 0;-- i) std::swap(S[i], S[rv.random_int(i+1)]); /* duplicate random_shuffle */
    }
    /**
     * check whether S is a perfect matching
     * @param S
     * @return
     */
    static bool is_matching(std::vector<int>& S)
    {
        int len = S.size();
        std::vector<int> visited(S.size(), LWS_UNMATCHED);
        bool result = true;

        for(int i = 0;i < len;++ i)
            if (S[i] >= 0 && S[i] < len) visited[S[i]] = LWS_MATCHED;
            else { result = false; break; }

        for (int i = 0;i < len;++ i)
            if (visited[i] != LWS_MATCHED) { result = false; break; }
        return result;
    }
    /**
     * check whether S is a valid matching
     * @param S
     * @return
     */
    static bool is_partial_matching(std::vector<int>& S){
        int len = S.size();
        std::vector<int> visited(len, LWS_UNMATCHED);
        bool result = true;

        int k;
        for(int i = 0;i < len;++ i)
            if (S[i] >= 0 && S[i] < len) {/* valid */
                k = S[i];
                if (visited[k] != LWS_MATCHED) visited[k] = LWS_MATCHED;
                else { result = false; break; }
            }

        return result;
    }
    /**
     * make a valid matching perfect
     * @param S
     */
    static void fix_matching(std::vector<int>& S)
    {
        int len = S.size();
        std::vector<int> is_matched(len, LWS_UNMATCHED);
        std::vector<int> unmatched_in;
        std::vector<int> unmatched_out;

        int i, j, k;
        // int unmatched_counter = 0;

        /*! find all unmatched inputs & mark all matched inputs */
        for (i = 0;i < len;++ i)
            if (S[i] != LWS_UNMATCHED) is_matched[S[i]] = LWS_MATCHED; /*! mark matched output */
            else unmatched_in.push_back(i); /*! record unmatched inputs */

        /*! find all unmatched outputs */
        for (k = 0;k < len;++ k)
            if (is_matched[k] == LWS_UNMATCHED) unmatched_out.push_back(k);

        assert(unmatched_in.size() == unmatched_out.size());

        /*! matched all the unmatched inputs and outputs (round-robin) */
        for (j = 0;j < unmatched_in.size();++ j) {
            i = unmatched_in[j];
            k = unmatched_out[j];
            S[i] = k;
        }
    }
    template <class T>
    static int weighted_selection(std::vector<int>& items,std::vector<T>& weights, RV& rv, unsigned long len=0)
    {
        if (len == 0) len = items.size();
        assert(len == weights.size());

        std::vector<T> cumsum(len, 0);

        int i, j = len - 1;
        double w;

        assert(weights[0] >= 0);
        cumsum[0] = weights[0];
        for (i = 1;i < len;++ i) {
            assert(weights[i] >= 0);
            cumsum[i] = cumsum[i - 1] + weights[i];
        }

        w = rv.random_01() * (double)cumsum[len - 1];

        for (i = 0;i < len;++ i){
            if (w < cumsum[i]){
                j = i;
                break;
            }
        }
        return items[j];
    }
    template <class T>
    static int weighted_selection(std::vector<T>& weights, RV& rv)
    {
        unsigned long len = weights.size();


        std::vector<T> cumsum(len, 0);

        int i, j = len - 1;
        double w;

        assert(weights[0] >= 0);
        cumsum[0] = weights[0];
        for (i = 1;i < len;++ i) {
            assert(weights[i] >= 0);
            cumsum[i] = cumsum[i - 1] + weights[i];
        }
        if (cumsum[len - 1] == 0) return -1;

        w = rv.random_01() * (double)cumsum[len - 1];

        for (i = 0;i < len;++ i){
            if (w < cumsum[i]){
                j = i;
                break;
            }
        }
        return j;
    }
    template <class T, class W>
    static int weighted_selection(std::vector<T>& weights, RV& rv, W* f)
    {
        assert(f->f(0) == 0 || f->f(0) == std::numeric_limits<double>::infinity());
        unsigned long len = weights.size();
        int i, j = len - 1, k;
        double w;

        if (f->f(0) == std::numeric_limits<double>::infinity()){/* infinity weight func */
            std::vector<int> rand_perm(len, 0);
            UT::rpermute(rand_perm, rv);

            w = 0;
            k = -1;
            for (std::vector<int>::iterator it=rand_perm.begin();it != rand_perm.end();++ it){
                i = *it;
               if (weights[i] > w){k = i; w = weights[i];}
            }
            return k;
        }


        std::vector<double> cumsum(len, 0);



        assert(f->f(weights[0]) >= 0);
        cumsum[0] = f->f(weights[0]);
        for (i = 1;i < len;++ i) {
            assert(f->f(weights[i]) >= 0);
            cumsum[i] = cumsum[i - 1] + f->f(weights[i]);
        }
        if (cumsum[len - 1] == 0) return -1;

        w = rv.random_01() * (double)cumsum[len - 1];

        for (i = 0;i < len;++ i){
            if (w < cumsum[i]){
                j = i;
                break;
            }
        }
        return j;
    }
    template <class T, class W>
    static int weighted_selection(std::vector<int> items, std::vector<T>& weights, RV& rv, W* f, unsigned long len = 0)
    {
        assert(f->f(0) == 0 || f->f(0) == std::numeric_limits<double>::infinity());
        if (len == 0) len = weights.size();
        int i, j = len - 1, k;
        double w;

        if (f->f(0) == std::numeric_limits<double>::infinity()){/* infinity weight func */
            std::vector<int> rand_perm(len, 0);
            UT::rpermute(rand_perm, rv);

            w = 0;
            k = -1;
            for (std::vector<int>::iterator it=rand_perm.begin();it != rand_perm.end();++ it){
                i = *it;
                if (weights[i] > w){k = i; w = weights[i];}
            }
            return k;
        }


        std::vector<double> cumsum(len, 0);



        assert(f->f(weights[0]) >= 0);
        cumsum[0] = f->f(weights[0]);
        for (i = 1;i < len;++ i) {
            assert(f->f(weights[i]) >= 0);
            cumsum[i] = cumsum[i - 1] + f->f(weights[i]);
        }
        if (cumsum[len - 1] == 0) return -1;

        w = rv.random_01() * (double)cumsum[len - 1];

        for (i = 0;i < len;++ i){
            if (w < cumsum[i]){
                j = i;
                break;
            }
        }
        return items[j];
    }
    static void merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S, std::vector<std::vector<int> >& weights) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */
        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;
            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];
            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];
                if (current == ibk) break; /* back to i */
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */
                }
            }
            if (w < 0) selected_matching_id[c] = 1;
            ++ c;
        }

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    static void merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S,
                      std::vector<std::vector<int> >& weights, std::vector<int>& Ouroboros) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;
            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];
            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];
                if (current == ibk) 
                {
                    break; /* back to i */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */
                }
            }
            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;

            unmatched_inputs_pre = unmatched_inputs;
            if (w < 0 && Ouroboros[cycle_length - 1] == 0) selected_matching_id[c] = 1;
            ++ c;
        }

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    static void merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S,
                      std::vector<std::vector<int> >& weights, std::vector<int>& Ouroboros,
                     int& cn_t, int& cn_o) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;
            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];
            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];
                if (current == ibk)
                {
                    break; /* back to i */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */
                }
            }
            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;

            unmatched_inputs_pre = unmatched_inputs;
            if (Ouroboros[cycle_length - 1] == 0) {
                selected_matching_id[c] = w < 0 ? 1:0;
                ++ cn_o;
            }
            ++ c;
        }
        cn_t += c;

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    static void merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S,
                      std::vector<std::vector<int> >& weights, std::vector<int>& Ouroboros, int(*decisionFunc)(const int x, const int y)) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        int leader;

        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;

            leader = i;/* leader init */

            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];

            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];

                leader = leader < current?leader:current;

                if (current == ibk)
                {
                    break; /* back to i */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */
                }
            }

            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;

            unmatched_inputs_pre = unmatched_inputs;
            if (Ouroboros[cycle_length - 1] == 0) selected_matching_id[c] = w < 0?1:0;
            else
            {
                int wr = 0, wb = 0;
                int cur_node = leader;
                // total weight
//                for (int r = 0;r <= static_cast<int>(std::ceil(std::log2(N)));++ r){
                for (int r = 0;r < N;++ r){
                    wr += weights[cur_node][S1[cur_node]];
                    wb += weights[S2_reverse[S1[cur_node]]][S1[cur_node]];
                    cur_node = S2_reverse[S1[cur_node]];
                }
                selected_matching_id[c] = decisionFunc(wr, wb) > 0?0:1;
            }

            ++ c;
        }

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    static void merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S,
                      std::vector<std::vector<int> >& weights, std::vector<int>& Ouroboros,
                      int(*decisionFunc)(const int x, const int y), int& cn_t, int& cn_o, int& cn_w) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        int leader;

        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;

            leader = i;/* leader init */

            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];

            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];

                leader = leader < current?leader:current;

                if (current == ibk)
                {
                    break; /* back to i */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */
                }
            }

            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;

            unmatched_inputs_pre = unmatched_inputs;
            if (Ouroboros[cycle_length - 1] == 0) {
                selected_matching_id[c] = w < 0?1:0;
                ++ cn_o;
            }
            else
            {
                int wr = 0, wb = 0;
                int cur_node = leader;
                // total weight
//                for (int r = 0;r <= static_cast<int>(std::ceil(std::log2(N)));++ r){
                for (int r = 0;r < N;++ r){
                    wr += weights[cur_node][S1[cur_node]];
                    wb += weights[S2_reverse[S1[cur_node]]][S1[cur_node]];
                    cur_node = S2_reverse[S1[cur_node]];
                }
                selected_matching_id[c] = decisionFunc(wr, wb) >= 0?0:1;

                cn_w += ((w < 0)?1:0) == selected_matching_id[c] ? 0:1;
            }

            ++ c;
        }
        cn_t += c;

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    static void merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S, std::vector<std::vector<int> >& weights, std::ostream& os) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */
        std::vector<int> cycle_length_recorder;

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;
            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];
            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];
                if (current == ibk) 
                {
                    break; /* back to i */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */
                }
            }
            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;
            cycle_length_recorder.push_back(cycle_length);
            unmatched_inputs_pre = unmatched_inputs;
            if (w < 0) selected_matching_id[c] = 1;
            ++ c;
        }

        // record to stream
        os << c << " ";
        for (auto cl: cycle_length_recorder) os << cl << " ";
        os << std::endl;

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
#if defined(Serenade) && defined(LWS_VERSION)
    /**
     * merge for C-serenade
     * @param S1                  -- matching from the previous time slot
     * @param S2                  -- arrival (populating to a full one)
     * @param S                   -- result matching
     * @param weights             -- queue lengths
     * @param Ouroboros           -- Ouroboros vector
     * @param instruments         -- instrument obj
     */
    static void C_merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S,
                      std::vector<std::vector<int> >& weights, std::vector<int>& Ouroboros,
                      lws_inst_t& instruments) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;
        int w1, w2;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;

            input_matched_counter[ibk] = c; // update input match counter (i.e., in which cycle it is matched)

            -- unmatched_inputs;// update unmatched input ports

            current = S1[i]; /* corresponding output */

            w = weights[i][current];

            // added on Back Friday 2016
            w1 = weights[i][current]; // weight of edges from S1
            w2 = 0;// weight of edges from S2

            for (j = 0;j < 2 * N;++ j) {

                k = current;
                current = S2_reverse[k];/* new input port */

                w -= weights[current][k];

                //
                w2 += weights[current][k];// weight of edges from S2

                if (current == ibk)
                {
                    break; /* back to i, i.e., cycle closed */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;

                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */

                    //
                    w1 += weights[i][current];// weight of edges from S1
                }
            }

            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;

            ++ instruments.C_size_hist[cycle_length - 1]; // update cycle size counter

            unmatched_inputs_pre = unmatched_inputs;

            if (Ouroboros[cycle_length - 1] == 0) {// Ouroboros cycle

                selected_matching_id[c] = (w < 0?1:0);

                // do instruments
                ++ instruments.C_counter_O; // update counter for Ouroboros cycle
                instruments.W_counter_O += (w < 0?w2:w1); // update weight of OC
                instruments.W_counter_T += (w < 0?w2:w1); // update weight of C
            }
            else
            {// Non-Ouroboros cycle, select the default one, i.e., S1
                if (w < 0) {// wrong decision
                    ++ instruments.C_counter_W;// update counter for wrong cycles
                    instruments.W_counter_W += w1; // update wrong weights (weight of NOC)
                }
                instruments.W_counter_T += w1; // update total weights (weight of C)
            }
            ++ c; // update cycle counter
        }

        instruments.C_counter_T += c; // update counter for total cycles

        // get result matching
        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        // release spaces
        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    /**
     *
     * @param S1
     * @param S2
     * @param S
     * @param weights
     * @param Ouroboros
     * @param Q_bound                               -- threhold for the trigger of transition from O to C
     * @param instruments
     */
    static void O_merge(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S,
                        std::vector<std::vector<int> >& weights, std::vector<int>& Ouroboros,
                        int Q_bound, lws_inst_t& instruments) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;
        int w1, w2;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        int leader;
        int max_weight; // for triggering O to C

        while (unmatched_inputs > 0) {

            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;

            leader = i;/* leader init for a cycle */
            max_weight = -1; /* initialize max weight */

            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];

            max_weight = (max_weight > weights[i][current] ? max_weight : weights[i][current]);

            // added on Back Friday 2016
            w1 = weights[i][current];
            w2 = 0;

            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];

                max_weight = (max_weight > weights[current][k] ? max_weight : weights[current][k]);

                //
                w2 += weights[current][k];

                leader = (leader < current ? leader : current); // only care about input ports

                if (current == ibk)
                {
                    break; /* back to i */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */

                    max_weight = (max_weight > weights[i][current] ? max_weight : weights[i][current]);

                    //
                    w1 += weights[i][current];
                }
            }

            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;

            ++ instruments.C_size_hist[cycle_length - 1]; // update cycle size counter

            unmatched_inputs_pre = unmatched_inputs;

            if (Ouroboros[cycle_length - 1] == 0) {
                selected_matching_id[c] = (w < 0 ? 1 : 0);
                //
                ++ instruments.C_counter_O; // Ouroboros cycle
                instruments.W_counter_O += (w < 0 ? w2 : w1); // weight of OC
                instruments.W_counter_T += (w < 0 ? w2 : w1); // weight of C
            }
            else
            {// Non-Ouroboros
                if (max_weight > Q_bound){// back to C_merge
                    if (w < 0) {// wrong decision
                        ++ instruments.C_counter_W;
                        instruments.W_counter_W += w1; // update wrong weights (weight of NOC)
                    }
                    instruments.W_counter_T += w1; // update total weights (weight of C)
                    ++ instruments.C_counter_BC; // update back to C_merge counter
                } else {// O_merge
                    int wr = 0, wb = 0;
                    int cur_node = leader;
                    // total weight
                    for (int r = 0;r < N;++ r) {
                        wr += weights[cur_node][S1[cur_node]];
                        wb += weights[S2_reverse[S1[cur_node]]][S1[cur_node]];
                        cur_node = S2_reverse[S1[cur_node]];
                    }

                    selected_matching_id[c] = (wr > wb ? 0 : 1);

                    int expected = (w < 0 ? 1 : 0);

                    if (expected != selected_matching_id[c]) {// wrong decision
                        ++ instruments.C_counter_W;
                        instruments.W_counter_W += (selected_matching_id[c] == 0 ? w1 : w2); // update wrong weights (weight of NOC)
                    }
                    instruments.W_counter_T += (selected_matching_id[c] == 0 ? w1 : w2); // update total weights (weight of C)
                }

            }

            ++ c; // update cycle counter
        }

        instruments.C_counter_T += c;

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    static void C_merge_v2(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S,
                        std::vector<std::vector<int> >& weights, std::vector<int>& Ouroboros,
                        lws_inst_t& instruments) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;
        int w1, w2;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        int ourobo_v = 0;

        while (unmatched_inputs > 0) {
            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;

            input_matched_counter[ibk] = c; // update input match counter (i.e., in which cycle it is matched)

            -- unmatched_inputs;// update unmatched input ports

            current = S1[i]; /* corresponding output */

            w = weights[i][current];

            // added on Back Friday 2016
            w1 = weights[i][current]; // weight of edges from S1
            w2 = 0;// weight of edges from S2

            for (j = 0;j < 2 * N;++ j) {

                k = current;
                current = S2_reverse[k];/* new input port */

                w -= weights[current][k];

                //
                w2 += weights[current][k];// weight of edges from S2

                if (current == ibk)
                {
                    break; /* back to i, i.e., cycle closed */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;

                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */

                    //
                    w1 += weights[i][current];// weight of edges from S1
                }
            }

            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;

            ++ instruments.C_size_hist[cycle_length - 1]; // update cycle size counter

            unmatched_inputs_pre = unmatched_inputs;

            if (Ouroboros[cycle_length - 1] == 0) {// Ouroboros cycle

                selected_matching_id[c] = (w < 0?1:0);

                // do instruments
                ++ instruments.C_counter_O; // update counter for Ouroboros cycle
                instruments.W_counter_O += (w < 0?w2:w1); // update weight of OC
                instruments.W_counter_T += (w < 0?w2:w1); // update weight of C


                // newly added
                ourobo_v += cycle_length;
                instruments.V_counter_O_woE += cycle_length;

            }
            else
            {// Non-Ouroboros cycle, select the default one, i.e., S1
                if (w < 0) {// wrong decision
                    ++ instruments.C_counter_W;// update counter for wrong cycles
                    instruments.W_counter_W += w1; // update wrong weights (weight of NOC)

                    instruments.V_counter_nonO_W_woE += cycle_length;
                }
                instruments.W_counter_T += w1; // update total weights (weight of C)
            }
            ++ c; // update cycle counter
        }

        instruments.V_counter_T_woE += N;

        if (ourobo_v == N) {// all ouroboros

            instruments.N_counter_AO_woE += 1;

        }
        instruments.N_counter_T_woE += 1;

        instruments.C_counter_T += c; // update counter for total cycles

        // get result matching
        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        // release spaces
        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    static void O_merge_v2(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S,
                        std::vector<std::vector<int> >& weights, std::vector<int>& Ouroboros,
                        int Q_bound, lws_inst_t& instruments) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;
        int w1, w2;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N, unmatched_inputs_pre = N;
        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        int cycle_length = 0;

        int leader;
        int max_weight; // for triggering O to C

        int ouroboros_v = 0;

        while (unmatched_inputs > 0) {

            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;

            leader = i;/* leader init for a cycle */
            max_weight = -1; /* initialize max weight */

            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];

            max_weight = (max_weight > weights[i][current] ? max_weight : weights[i][current]);

            // added on Back Friday 2016
            w1 = weights[i][current];
            w2 = 0;

            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];

                max_weight = (max_weight > weights[current][k] ? max_weight : weights[current][k]);

                //
                w2 += weights[current][k];

                leader = (leader < current ? leader : current); // only care about input ports

                if (current == ibk)
                {
                    break; /* back to i */
                }
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */

                    max_weight = (max_weight > weights[i][current] ? max_weight : weights[i][current]);

                    //
                    w1 += weights[i][current];
                }
            }

            // add on Oct. 27
            // cycle length
            cycle_length = unmatched_inputs_pre - unmatched_inputs;

            ++ instruments.C_size_hist[cycle_length - 1]; // update cycle size counter

            unmatched_inputs_pre = unmatched_inputs;


            if (Ouroboros[cycle_length - 1] == 0) { // ouro
                selected_matching_id[c] = (w < 0 ? 1 : 0);
                //
                ++ instruments.C_counter_O; // Ouroboros cycle
                instruments.W_counter_O += (w < 0 ? w2 : w1); // weight of OC
                instruments.W_counter_T += (w < 0 ? w2 : w1); // weight of C

                // added @01/07 2017
                instruments.V_counter_O_woE += cycle_length;
                ouroboros_v += cycle_length;
            }
            else
            {// Non-Ouroboros
                if (max_weight > Q_bound){// back to C_merge
                    if (w < 0) {// wrong decision
                        ++ instruments.C_counter_W;
                        instruments.W_counter_W += w1; // update wrong weights (weight of NOC)

                        // update vertices counter
                        instruments.V_counter_nonO_W_woE += cycle_length;
                    }

                    instruments.W_counter_T += w1; // update total weights (weight of C)
                    ++ instruments.C_counter_BC; // update back to C_merge counter
                } else {// O_merge

                    int wr = 0, wb = 0;
                    int cur_node = leader;
                    // total weight
                    for (int r = 0;r < N;++ r) {
                        wr += weights[cur_node][S1[cur_node]];
                        wb += weights[S2_reverse[S1[cur_node]]][S1[cur_node]];
                        cur_node = S2_reverse[S1[cur_node]];
                    }

                    selected_matching_id[c] = (wr > wb ? 0 : 1);

                    int expected = (w < 0 ? 1 : 0);

                    if (expected != selected_matching_id[c]) {// wrong decision
                        ++ instruments.C_counter_W;
                        instruments.W_counter_W += (selected_matching_id[c] == 0 ? w1 : w2); // update wrong weights (weight of NOC)

                        // update vertices counter
                        instruments.V_counter_nonO_W_woE += cycle_length;
                    }
                    instruments.W_counter_T += (selected_matching_id[c] == 0 ? w1 : w2); // update total weights (weight of C)

                    instruments.C_counter_nonO_woE += 1; // non-ouroboros cycle in O-SERENADE
                }

            }

            ++ c; // update cycle counter
        }

        instruments.V_counter_T_woE += N;

        if (ouroboros_v == N) {// all cycles ouroboros

            instruments.N_counter_AO_woE += 1;

        }

        instruments.N_counter_T_woE += 1; // update total ts counter in C/O-SERENADE

        instruments.C_counter_T += c;

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
    static void E_merge_v1(std::vector<int>& S1, std::vector<int>& S2, std::vector<int>& S, std::vector<std::vector<int> >& weights,
                           std::vector<int>& BSIT, lws_inst_t& instruments) {

        assert(is_matching(S1) && is_matching(S2));
        assert((S1.size() == S2.size()) && (S1.size() == S.size()));

        int N = S1.size();
        int i, k, j, c, w, ibk;

        std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

        for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

        int unmatched_inputs = N;

        // newly added
        int unmatched_inputs_pre = N;
        int cycle_length = 0;
        int counter_itmp = 0;


        int current = 0;

        std::vector<int> input_matched_counter(N, -1);
        std::vector<int> selected_matching_id(N, 0);

        c = 0;/* cycle counter */

        while (unmatched_inputs > 0) {


            for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
            ibk = i;
            input_matched_counter[ibk] = c;
            -- unmatched_inputs;
            current = S1[i]; /* corresponding output */
            w = weights[i][current];
            for (j = 0;j < 2 * N;++ j) {
                k = current;
                current = S2_reverse[k];/* new input port */
                w -= weights[current][k];
                if (current == ibk) break; /* back to i */
                else {
                    i = current;
                    input_matched_counter[i] = c; /* add label */
                    -- unmatched_inputs;
                    current = S1[i];/* output port */
                    w += weights[i][current];/* update weight difference */
                }
            }
            if (w < 0) selected_matching_id[c] = 1;


            ++ c;

            cycle_length = unmatched_inputs_pre - unmatched_inputs;

            counter_itmp += BSIT[cycle_length - 1];

            instruments.C_size_hist_for_E[cycle_length - 1] += 1;

            if (BSIT[cycle_length - 1] > 0) {// broadcast size increases

                instruments.C_counter_nonO_E += 1;
            }

            unmatched_inputs_pre = unmatched_inputs;
        }

        if (counter_itmp > 0) {// not all ouroboros

            instruments.N_counter_nonO_E += 1;
            instruments.BSIT_counter_E += counter_itmp;

        }

        instruments.N_counter_TE += 1;

        for (i = 0;i < N;++ i) {
            c = input_matched_counter[i]; /* in which iteration this input is visited */
            assert(c != -1);
            S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
        }

        S2_reverse.clear();
        input_matched_counter.clear();
        selected_matching_id.clear();
        assert(UT::is_matching(S)); /*! merged match should be a full matching */
    }
#endif
};

class Scheduler {
public:
    int port_num;
    Scheduler(int p): port_num(p) {}


#if defined(Serenade) && defined(LWS_VERSION)
    virtual void run(lws_param_t &g_params, lws_status_t &g_status, lws_inst_t& g_instruments) = 0;
#else
    virtual void run(lws_param_t &g_params, lws_status_t &g_status) = 0;
#endif
    virtual void reset() = 0;
};


class iScheduler: virtual public Scheduler {
public:
    typedef std::vector<std::vector<int> > Matrix;
    std::vector<int> input_matched;
    std::vector<int> output_matched;
    std::vector<int> check;
    int rounds;

    iScheduler(int p): Scheduler(p), input_matched(p, 0), output_matched(p, 0), check(p, 0){
        rounds = static_cast<int>(std::ceil(std::log2(p)));
    }

#if defined(Serenade) && defined(LWS_VERSION)
    virtual void run(lws_param_t &g_params, lws_status_t &g_status, lws_inst_t& g_instruments) = 0;
#else
    virtual void run(lws_param_t &g_params, lws_status_t &g_status) = 0;
#endif
    virtual void reset() = 0;
};


void single_run(LWS_SwitchCore& sw, RV &rv, Scheduler *sch, std::ostream& results){
    if (sw.params.model_option == LWS_ALL_MODELS) {std::cerr << "Can not run all models in single run" << std::endl; return;}
    if (sw.params.verbosity >= 1) sw.print_all();

    TrafficGenerator *tgen = NULL;

    if (sw.params.inject_option == LWS_ON_OFF_BURST)
        tgen = new BurstTrafficGenerator(sw.params.N, sw.params.load, sw.params.model_option,
                                         sw.params.average_burst_length); /* traffic generator */
    else
        tgen = new TrafficGenerator(sw.params.N, sw.params.load, sw.params.model_option);


    for (int ts = 0; ts < sw.params.MAX_M; ++ts) {
        /* set time slot */
        sw.status.cur_time = ts;

        SwitchEvent::packet_arrival(sw.params, sw.status, sw.instruments, tgen, rv);

        /* put scheduling here */
#if defined(Serenade) && defined(LWS_VERSION)
        sch->run(sw.params, sw.status, sw.instruments);
#else
        sch->run(sw.params, sw.status);
#endif

        SwitchEvent::packet_departure(sw.params, sw.status, sw.instruments);
        /* put instruments printing here if necessary */
        if (sw.params.MAX_M - 1 == ts) sw.show(results);
        if ((ts > 0 && (ts % (sw.params.MAX_M / LWS_OUT_FREQ) == 0)) || ts == sw.params.MAX_M - 1) sw.show(std::cout,ts == sw.params.MAX_M - 1);
    }
    if (sw.params.verbosity >= 1)
        hdr_percentiles_print(
                sw.instruments.histogram,
                stdout,  // File to write to
                5,  // Granularity of printed values
                1.0,  // Multiplier for results
                CLASSIC);

    sw.reset();/* reset */
    sch->reset(); /* reset schedule */

    assert(tgen != NULL);
    delete tgen;
}

void main_run(LWS_SwitchCore& sw, RV& rv, Scheduler *sch, std::ostream& results){

    sw.show_header(results, false);

    if (sw.params.inject_option == LWS_ON_OFF_BURST) {/* burst traffic */
        for (auto load: sw.params.loads) {/* each load */
            for (auto bs: sw.params.abls) {/* each burst size */
                sw.params.load = load;
                sw.params.average_burst_length = bs;
                if (sw.params.model_option == LWS_ALL_MODELS) {/* all models */

                    for (int model = LWS_UNIFORM; model <= LWS_DIAGONAL; ++model) {
                        sw.params.model_option = model;
                        single_run(sw, rv, sch, results);
                    }

                    sw.params.model_option = LWS_ALL_MODELS;
                } else {
                    single_run(sw, rv, sch, results);
                }
            }
        }
    }else{
        for (auto load: sw.params.loads) {/* each load */
            sw.params.load = load;
            if (sw.params.model_option == LWS_ALL_MODELS) {
                for (int model = LWS_UNIFORM;model <= LWS_DIAGONAL;++ model) {
                    sw.params.model_option = model;
                    single_run(sw, rv, sch, results);
                }
                sw.params.model_option = LWS_ALL_MODELS;

            }else {
                single_run(sw, rv, sch, results);
            }
        }

    }
}


#endif //LWS_H
