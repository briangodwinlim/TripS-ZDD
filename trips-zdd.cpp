#include <map>
#include <vector>
#include <iomanip>

// SAPPOROBDD
#include <ZBDD.h>
#include <SBDD_helper.h>

// TdZdd
#include <tdzdd/DdEval.hpp>
#include <tdzdd/DdStructure.hpp>
#include <tdzdd/spec/SapporoZdd.hpp>
#include <tdzdd/eval/ToZBDD.hpp>

#include "GetConfig.hpp"
#include "ValidConfig.hpp"
#include "WeightedIterator.hpp"
#include "GeoDistributedStorageSystem.hpp"

typedef GeoDistributedStorageSystem::Cost Cost;
typedef std::map<std::string,std::vector<std::string>> TLL;
typedef std::pair<Cost,TLL> CostConfigPair;

using namespace tdzdd;
using namespace sbddh;

std::string options[][2] = { //
        {"dcList", "Input GDSS instance from STDIN"}, //
        {"strongSLA", "Strong consistency in latency SLA constraint"}, //
        {"openMP", "Use openMP in construction of ZDD"}, //                     OMP_NUM_THREADS=THREADS
        {"zdd", "Dump resulting ZDD to STDOUT in DOT format"}, //
        {"export", "Dump resulting ZDD to STDOUT"}, //
        {"getconfig <n>", "Get the first n optimal data placements"} //
    };

std::map<std::string,bool> opt;
std::map<std::string,int> optNum;

void usage(char const* cmd) {
    std::cerr << "usage:" << cmd << " [ <cost_info> <monitoring_info> <query> <goals> ] [ <options>... ] \n";
    std::cerr << "options\n";
    for (unsigned i = 0; i < sizeof(options) / sizeof(options[0]); ++i) {
        std::cerr << "  -" << options[i][0];
        for (unsigned j = options[i][0].length(); j < 15; ++j) {
            std::cerr << " ";
        }
        std::cerr << ": " << options[i][1] << "\n";
    }
}

int main(int argc, char *argv[]) {
    for (unsigned i = 0; i < sizeof(options) / sizeof(options[0]); ++i) {
        opt[options[i][0]] = false;
    }

    std::string cost_info, monitoring_info, query, goals;
    try {
        for (int i = 1; i < argc; ++i) {
            std::string s = argv[i];
            if (s[0] == '-') {
                s = s.substr(1);

                if (opt.count(s)) {
                    opt[s] = true;
                }
                else if (i + 1 < argc && opt.count(s + " <n>")) {
                    opt[s] = true;
                    optNum[s] = std::stoi(argv[++i]);
                }
                else {
                    throw std::exception();
                }
            }
            else if (cost_info.empty()) {
                cost_info = s;
            }
            else if (monitoring_info.empty()) {
                monitoring_info = s;
            }
            else if (query.empty()) {
                query = s;
            }
            else if (goals.empty()) {
                goals = s;
            }
            else {
                throw std::exception();
            }
        }
    }
    catch (std::exception& e) {
        usage(argv[0]);
        return 1;
    }

    MessageHandler::showMessages();
    MessageHandler mh;
    mh.begin("Started");

    GeoDistributedStorageSystem gdss;
    try {
        if (!goals.empty()) {
            // Read GDSS from JSON files
            gdss.readJSON(cost_info, monitoring_info, query, goals);
        }
        else if (opt["dcList"]) {
            // Set GDSS instance from STDIN
            mh << "\nInput dcList: ";
            std::string line;
            getline(std::cin, line);
            if (line.size() == 0) {
                usage(argv[0]);
                return 1;
            }
            std::vector<int> dcList;
            std::istringstream stream(line);
            for (int num; stream >> num; ) {
                dcList.push_back(num);
            }
            gdss.setInstance(dcList);
            mh << "Goals: F = " << gdss.getF() << ", LC = " << gdss.getLC() << "\n";
        }
        else {
            // Default GDSS
            gdss.setInstance();
            mh << "\nGoals: F = " << gdss.getF() << ", LC = " << gdss.getLC() << "\n";
        }           

        // Check all information in GDSS
        gdss.checkAll();

        // Determine latency SLA constraint
        std::string slaOption = opt["strongSLA"] ? "strong" : "eventual";

        // Run ValidConfig
        ValidConfig spec(gdss, slaOption);
        DdStructure<2> dd(spec, opt["openMP"]);
        dd.zddReduce();

        // Output ZDD information
        std::string cardinality = dd.evaluate(ZddCardinality<>());
        CostConfigPair optConfig = dd.evaluate(GetConfig(gdss));
        TLL targetLocaleList = optConfig.second;
        Cost currCost = optConfig.first;
        if (cardinality == "0") currCost = 0;
        mh << "\n#variable = " << spec.numVariables()
            << ", #node = " << dd.size() 
            << ", #solution = " << cardinality
            << ", Minimum cost = " << std::fixed << std::setprecision(10) << currCost
            << "\n";

        // Get the first n optimal data placements
        if (opt["getconfig"]) {
            if (cardinality == "0") {
                mh << "No solutions found\n";
                return 0;
            }

            MessageHandler config;
            config.begin("Finding optimal configurations");

            // Convert to ZBDD and get WeightedIterator
            BDD_Init(10000, 8000000000LL);
            for (int i = 0; i < dd.topLevel(); ++i) BDD_NewVar();
            ZBDD dd_s = dd.evaluate(ToZBDD());
            weighted_iterator<Cost> it(dd_s, getCost(gdss), false);
            
            // Go through ZDD
            std::map<int,std::string> suffix = {{1,"st"}, {2,"nd"}, {3,"rd"}};
            for (int n = 1; n <= optNum["getconfig"]; ++n) {
                targetLocaleList = to_TLL(gdss, *it);
                currCost = it.curr_weight();

                // Print optimal placements
                if (suffix.count(n) == 0) suffix[n] = "th";
                std::cout << "\n" << n << suffix[n] << " Best Placement\n";
                std::cout << "Data Placement\n";
                for (int i = 0; i < targetLocaleList["storageTiers"].size(); ++i) 
                    std::cout << targetLocaleList["storageTiers"][i] << " ";
                std::cout << "\n\n";
                std::cout << "Target Locale List\n";
                for (auto const& imap: targetLocaleList) {
                    if (imap.first == "storageTiers") continue;
                    std::cout << imap.first << " -> ";
                    for (auto storageTier: imap.second) std::cout << storageTier << " ";
                    std::cout << "\n";
                }
                std::cout << "\nCurrent Cost = " << std::setprecision(10) << currCost << "\n";   
                it.next();
            }

            config.end("finished");
        }

        // Output resulting ZDD
        if (opt["zdd"] && dd.size()) dd.dumpDot(std::cout, "ZDD");
        if (opt["export"] && dd.size()) dd.dumpSapporo(std::cout);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    mh.end("finished");
    return 0;
}