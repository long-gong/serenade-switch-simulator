#include "Serena.hpp"


int main(int argc, char* argv[]){
    LWS_SwitchCore sw(argv[0]);
    sw.init(argc, argv);
    // obtain a seed from the system clock:
    unsigned seed1 = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());
 
    std::string fn = "../results/Serena-" + std::to_string(sw.params.N) +
                     "-" + std::to_string(sw.params.MAX_M) + "-" + std::to_string(seed1);
    if (sw.params.inject_option == LWS_ON_OFF_BURST){/* burst */
        fn += "-burst";
    }
    fn += ".dat";

    std::ofstream results(
            fn,
            std::ofstream::out
    );

    if (results.fail()){// open failed
        std::cerr << "Open file " << fn << " failed" << std::endl;
        exit(1);
    }

    Serena serena_s(sw.params.N); /* create a Serena scheduler */

    RV rv_for_tgen(sw.params.seed);

    main_run(sw, rv_for_tgen, &serena_s, results);

    sw.destroy(); /* pay attention */
    results.close(); /* close results file */

    return 0;
}