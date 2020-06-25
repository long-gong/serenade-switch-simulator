//
// Created by root on 10/1/16.
//

// TODO
// make the whole process as general code, i.e., changing
// scheduler does not need to duplicate the whole code
//

#include "iSLIP.hpp"
#include <chrono>


int main(int argc, char* argv[]){

    LWS_SwitchCore sw(argv[0]);
    sw.init(argc, argv);
    // obtain a seed from the system clock:
    unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
 
    std::string fn = "../results/iSLIP-" + std::to_string(sw.params.N) +
                     "-" + std::to_string(sw.params.MAX_M) + "-" + std::to_string(seed1);
    if (sw.params.inject_option == LWS_ON_OFF_BURST){/* burst */
        fn += "-burst";
    }
    fn += ".dat";

    std::ofstream results(
            fn,
            std::ofstream::out
    );

    iSLIP islip_s(sw.params.N); /* create a iSLIP scheduler */

    RV rv_for_tgen(sw.params.seed);

    main_run(sw, rv_for_tgen, &islip_s, results);

    sw.destroy(); /* pay attention */
    results.close(); /* close results file */

    return 0;

}


