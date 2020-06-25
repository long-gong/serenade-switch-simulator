#include "Serenade_Opportunistic_Proof.hpp"
#include <fstream>

int main(int argc, char* argv[]){
    LWS_SwitchCore sw(argc, argv);

    // obtain a seed from the system clock:
    unsigned seed1 = static_cast<unsigned >(std::chrono::system_clock::now().time_since_epoch().count());
 
    std::string fn = "../results/O_Serenade_Proof-" + std::to_string(sw.params.N) +
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


    std::string Ouroboros_filename = sw.params.Ouroboros_filename;
    std::ifstream Ouroboros_file(
                                 Ouroboros_filename,
                                 std::ifstream::in
                                 );

    if (Ouroboros_file.fail()){// open failed
        std::cerr << "Can not open file " << Ouroboros_filename << std::endl;
        exit(1);
    }

    std::vector<int> Ouroboros(sw.params.N, 0);
    for (int i = 0;i < Ouroboros.size();++ i) Ouroboros_file >> Ouroboros[i];

    if (sw.params.verbosity >= 2) for (auto v: Ouroboros) std::cout << v << " ";
    std::cout << std::endl;


    E_Serenade_ serenade_opt_s(sw.params.N, Ouroboros, sw.params.Q_bound, sw.params.frequency); /* create a Serena scheduler */

    RV rv_for_tgen(sw.params.seed);

    main_run(sw, rv_for_tgen, &serenade_opt_s, results);

    sw.destroy(); /* pay attention */
    results.close(); /* close results file */
    Ouroboros_file.close(); /* close Ouroboros file */

    return 0;
}