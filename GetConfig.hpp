/*
 * A DdEval for getting an optimal configuration/placement for the
 * Geo-Distributed Multi-Cloud Data Center Storage Tiering and Selection Problem
 */

#pragma once

#include <set>
#include <map>
#include <vector>

#include <tdzdd/DdEval.hpp>

#include "GeoDistributedStorageSystem.hpp"

typedef GeoDistributedStorageSystem::Cost Cost;
typedef std::map<std::string,std::vector<std::string>> TLL;
typedef std::pair<Cost,TLL> CostConfigPair;

namespace tdzdd {

class GetConfig: public DdEval<GetConfig,CostConfigPair> {
private:    
    GeoDistributedStorageSystem const& gdss;
    int const Pwidth;
    int const Twidth;
    int const numDC;
    int const numST;
    int const n;

    TLL validTLL {{"storageTiers", std::vector<std::string>()}};
    TLL invalidTLL;

public:
    GetConfig(GeoDistributedStorageSystem const& gdss) 
        : gdss(gdss), numDC(gdss.getNumDataCenters()), numST(gdss.getNumStorageTiers()),
          Pwidth(1 + gdss.getNumDataCenters() + gdss.getNumDataCenters() * gdss.getNumDataCenters()),
          Twidth(1 + gdss.getNumDataCenters()), 
          n(gdss.getNumStorageTiers() * (1 + gdss.getNumDataCenters() + gdss.getNumDataCenters() * gdss.getNumDataCenters())) {
    }

    void evalTerminal(CostConfigPair &v, int id) {
        v = id ? std::make_pair(0, validTLL) : std::make_pair(INT_MAX / 2, invalidTLL);
    }

    void evalNode(CostConfigPair &v, int level, DdValues<CostConfigPair,2> const& values) {
        assert(1 <= level && level <= n);
        int invLevel = n - level;
        
        // P_{kt}
        if (invLevel % Pwidth == 0) {
            int t = invLevel / Pwidth;
            int k = gdss.getIdxDataCenters(gdss.getStorageTiers(t, "dataCenter"));
            std::string DCk = gdss.getStorageTiers(t, "dataCenter");
            std::string STt = gdss.getStorageTiers(t, "storageTier");

            Cost currCost = gdss.getSize(DCk) * gdss.getStorageCost(DCk, STt);
            if (values.get(0).first > values.get(1).first + currCost) {
                v = values.get(1);
                v.first += currCost;
                v.second["storageTiers"].push_back("{" + DCk + ", " + STt + "}");
            }
            else {
                v = values.get(0);
            }
        }
        // T_{jkt}
        else if ((invLevel % Pwidth - 1) % Twidth == 0) {
            int t = invLevel / Pwidth;
            int k = gdss.getIdxDataCenters(gdss.getStorageTiers(t, "dataCenter"));
            int j = (invLevel % Pwidth - 1) / Twidth;
            std::string DCk = gdss.getStorageTiers(t, "dataCenter");
            std::string STt = gdss.getStorageTiers(t, "storageTier");
            std::string DCj = gdss.getDataCenters(j);

            Cost currCost = (gdss.getGetRequest(DCj) * (gdss.getSize(DCj) * (gdss.getNetworkCost(DCk, DCj) + gdss.getRetrieveCost(DCk, STt)) + gdss.getGetCost(DCk, STt)) +
                                gdss.getPutRequest(DCj) * (gdss.getSize(DCj) * (gdss.getNetworkCost(DCj, DCk) + gdss.getWriteCost(DCk, STt)) + gdss.getPutCost(DCk, STt)));
            if (values.get(0).first > values.get(1).first + currCost) {
                v = values.get(1);
                v.first += currCost;
                if (v.second.count(DCj) == 0) v.second[DCj] = std::vector<std::string>();
                v.second[DCj].push_back("{" + DCk + ", " + STt + "}"); 
            }
            else {
                v = values.get(0);
            }
        }
        // B_{ijkt}
        else {
            int t = invLevel / Pwidth;
            int k = gdss.getIdxDataCenters(gdss.getStorageTiers(t, "dataCenter"));
            int j = (invLevel % Pwidth - 1) / Twidth;
            int i = (invLevel % Pwidth - 1) % Twidth - 1;
            std::string STt = gdss.getStorageTiers(t, "storageTier");
            std::string DCk = gdss.getStorageTiers(t, "dataCenter");
            std::string DCj = gdss.getDataCenters(j);
            std::string DCi = gdss.getDataCenters(i);
            
            Cost currCost = gdss.getPutRequest(DCi) * (gdss.getSize(DCi) * (gdss.getNetworkCost(DCj, DCk) + gdss.getWriteCost(DCk, STt)) + gdss.getPutCost(DCk, STt));
            if (values.get(0).first > values.get(1).first + currCost) {
                v = values.get(1);
                v.first += currCost;
            }
            else {
                v = values.get(0);
            }
        }
    }
};

TLL to_TLL(GeoDistributedStorageSystem const& gdss, std::set<int> config) {
    int numDC = gdss.getNumDataCenters();
    int numST = gdss.getNumStorageTiers();
    int Pwidth = 1 + numDC + numDC * numDC;
    int Twidth = 1 + numDC;
    int n = numST * (1 + numDC + numDC * numDC);
    TLL targetLocaleList {{"storageTiers", std::vector<std::string>()}};

    for (int level: config) {
        int invLevel = n - level;
        // P_{kt}
        if (invLevel % Pwidth == 0) {
            int t = invLevel / Pwidth;
            int k = gdss.getIdxDataCenters(gdss.getStorageTiers(t, "dataCenter"));
            std::string DCk = gdss.getStorageTiers(t, "dataCenter");
            std::string STt = gdss.getStorageTiers(t, "storageTier");

            targetLocaleList["storageTiers"].push_back("{" + DCk + ", " + STt + "}");
        }
        // T_{jkt}
        else if ((invLevel % Pwidth - 1) % Twidth == 0) {
            int t = invLevel / Pwidth;
            int k = gdss.getIdxDataCenters(gdss.getStorageTiers(t, "dataCenter"));
            int j = (invLevel % Pwidth - 1) / Twidth;
            std::string DCk = gdss.getStorageTiers(t, "dataCenter");
            std::string STt = gdss.getStorageTiers(t, "storageTier");
            std::string DCj = gdss.getDataCenters(j);
            
            if (targetLocaleList.count(DCj) == 0) targetLocaleList[DCj] = std::vector<std::string>();
            targetLocaleList[DCj].push_back("{" + DCk + ", " + STt + "}"); 
        }
    }

    return targetLocaleList;
}

std::vector<Cost> getCost(GeoDistributedStorageSystem const& gdss) {
    int numDC = gdss.getNumDataCenters();
    int numST = gdss.getNumStorageTiers();
    int Pwidth = 1 + numDC + numDC * numDC;
    int Twidth = 1 + numDC;
    int n = numST * (1 + numDC + numDC * numDC);
    std::vector<Cost> costList(n + 1);
    Cost currCost;

    for (int level = 1; level <= n; ++level) {
        int invLevel = n - level;
        // P_{kt}
        if (invLevel % Pwidth == 0) {
            int t = invLevel / Pwidth;
            int k = gdss.getIdxDataCenters(gdss.getStorageTiers(t, "dataCenter"));
            std::string DCk = gdss.getStorageTiers(t, "dataCenter");
            std::string STt = gdss.getStorageTiers(t, "storageTier");
            currCost = gdss.getSize(DCk) * gdss.getStorageCost(DCk, STt);
        }
        // T_{jkt}
        else if ((invLevel % Pwidth - 1) % Twidth == 0) {
            int t = invLevel / Pwidth;
            int k = gdss.getIdxDataCenters(gdss.getStorageTiers(t, "dataCenter"));
            int j = (invLevel % Pwidth - 1) / Twidth;
            std::string DCk = gdss.getStorageTiers(t, "dataCenter");
            std::string STt = gdss.getStorageTiers(t, "storageTier");
            std::string DCj = gdss.getDataCenters(j);
            currCost = (gdss.getGetRequest(DCj) * (gdss.getSize(DCj) * (gdss.getNetworkCost(DCk, DCj) + gdss.getRetrieveCost(DCk, STt)) + gdss.getGetCost(DCk, STt)) +
                        gdss.getPutRequest(DCj) * (gdss.getSize(DCj) * (gdss.getNetworkCost(DCj, DCk) + gdss.getWriteCost(DCk, STt)) + gdss.getPutCost(DCk, STt)));
        }
        // B_{ijkt}
        else {
            int t = invLevel / Pwidth;
            int k = gdss.getIdxDataCenters(gdss.getStorageTiers(t, "dataCenter"));
            int j = (invLevel % Pwidth - 1) / Twidth;
            int i = (invLevel % Pwidth - 1) % Twidth - 1;
            std::string STt = gdss.getStorageTiers(t, "storageTier");
            std::string DCk = gdss.getStorageTiers(t, "dataCenter");
            std::string DCj = gdss.getDataCenters(j);
            std::string DCi = gdss.getDataCenters(i);
            currCost = gdss.getPutRequest(DCi) * (gdss.getSize(DCi) * (gdss.getNetworkCost(DCj, DCk) + gdss.getWriteCost(DCk, STt)) + gdss.getPutCost(DCk, STt));
        }

        costList[level] = currCost;
    }

    return costList;
}

} // namespace tdzdd