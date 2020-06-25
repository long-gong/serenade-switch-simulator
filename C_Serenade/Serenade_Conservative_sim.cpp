#include "Serenade_Conservative.hpp"
#include <fstream>

/**
 * Major changes:
 *  1. add file open failure check
 *
 */

int main(int argc, char *argv[]) {
  LWS_SwitchCore sw(argc, argv);

  // obtain a seed from the system clock:
  unsigned seed1 = static_cast<unsigned>(
      std::chrono::system_clock::now().time_since_epoch().count());

  std::string fn = "../results/C_Serenade-" + std::to_string(sw.params.N) +
                   "-" + std::to_string(sw.params.MAX_M) + "-" +
                   std::to_string(seed1);
  if (sw.params.inject_option == LWS_ON_OFF_BURST) { /* burst */
    fn += "-burst";
  }
  fn += ".dat";

  std::ofstream results(fn, std::ofstream::out);

  if (results.fail()) { // open failed
    std::cerr << "Open file " << fn << " failed" << std::endl;
    exit(1);
  }

  std::string Ouroboros_filename = sw.params.Ouroboros_filename;
  std::ifstream Ouroboros_file(Ouroboros_filename, std::ifstream::in);

  if (Ouroboros_file.fail()) { // open failed
    std::cerr << "Can not open file " << Ouroboros_filename << std::endl;
    exit(1);
  }
  std::vector<int> Ouroboros(sw.params.N, 0);
  for (int i = 0; i < Ouroboros.size(); ++i)
    Ouroboros_file >> Ouroboros[i];

  if (sw.params.verbosity >= 2)
    for (auto v : Ouroboros)
      std::cout << v << " ";
  std::cout << std::endl;

  Serenade_Conservative_for_Proof serenade_cons_s(
      sw.params.N, Ouroboros); /* create a Serena scheduler */

  RV rv_for_tgen(sw.params.seed);

  main_run(sw, rv_for_tgen, &serenade_cons_s, results);

  sw.destroy();           /* pay attention */
  results.close();        /* close results file */
  Ouroboros_file.close(); /* close Ouroboros file */

  return 0;
}