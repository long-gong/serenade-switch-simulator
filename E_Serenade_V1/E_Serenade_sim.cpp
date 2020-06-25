#include "E_Serenade.hpp"
#include <fstream>

int main(int argc, char* argv[]){
    LWS_SwitchCore sw(argc, argv);

    // obtain a seed from the system clock:
    unsigned seed1 = static_cast<unsigned >(std::chrono::system_clock::now().time_since_epoch().count());
 
    std::string fn = "../results/E_Serenade-" + std::to_string(sw.params.N) +
                     "-" + std::to_string(sw.params.MAX_M) + "-" + std::to_string(seed1);
    if (sw.params.inject_option == LWS_ON_OFF_BURST){/* burst */
        fn += "-burst";
    }
    fn += ".dat";

    std::ofstream results(
            fn,
            std::ofstream::out
    );

    if (results.fail()) {
        std::cerr << "Open file " << fn << " failed" << std::endl;
        exit(1);
    }


    std::vector<int> BSIT(sw.params.N, 0);

    std::ifstream BSIT_file(
            "../data/BSIT/BSIT_" + std::to_string(sw.params.N) + ".txt",
            std::ifstream::in
    );

    if (BSIT_file.fail()) {

        std::cerr << "Can not open BSIT file" << std::endl;
        exit(1);

    }

    int tmp_ignore;
    for (auto &it: BSIT) BSIT_file >> tmp_ignore >> it;
    if (sw.params.verbosity >= 1) for (auto it: BSIT) std::cout << it << " ";
    std::cout << std::endl;
    BSIT_file.close();


    E_Serenade serenade_exact_s(sw.params.N, BSIT); /* create a Serena scheduler */

    RV rv_for_tgen(sw.params.seed);

    main_run(sw, rv_for_tgen, &serenade_exact_s, results);

    sw.destroy(); /* pay attention */
    results.close(); /* close results file */

    return 0;
}