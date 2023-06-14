/* 
 A class for Geo-Distributed Storage System (GDSS)
 
 Functions to query GDSS specifications
 ===============================================================================================================
 * addStorageTier(dataCenter, storageTier)              - add storageTier to dataCenter
 * update()                                             - update mapping from Data Centers and Storage Tiers to int
  
 * getDataCenters()                                     - list of Data Centers (list of str)
 * getDataCenters(idx)                                  - name of idx-th Data Center (str)
 * getNumDataCenters()                                  - number of Data Centers (int)
 * getIdxDataCenters(dataCenter)                        - index of dataCenter in list of Data Centers (int)
  
 * getStorageTiers(dataCenter)                          - list of Storage Tiers in dataCenter (list of str)
 * getStorageTiers(idx, option)                         - dataCenter/storageTier of idx-th Storage Tier (str); option = "dataCenter" or "storageTier"
 * getNumStorageTiers()                                 - number of Storage Tiers (int)
 * getNumStorageTiers(dataCenter)                       - number of Storage Tiers in dataCenter (int)
 * getNumStorageTiers(idx)                              - number of Storage Tiers in idx-th Data Center (int)
 * getIdxStorageTiers(dataCenter, storageTier, option)  - index of (dataCenter, storageTier) (int); option = "all" or "dataCenter"
 
 * readJSON(cost_info, monitoring_info, query, goals)   - set up gdss instance from JSON files
 * setInstance(dcList)                                  - set up a random gdss instance from a list of Storage Tiers dcList
 */

#pragma once

#include <set>
#include <map>
#include <vector>
#include <random>
#include <nlohmann/json.hpp>

typedef nlohmann::json json;

class GeoDistributedStorageSystem {
public:
    typedef long double Cost;
    typedef long double Size;
    typedef long double Latency;
    typedef long double Request;

private:
    std::vector<std::string> dataCenters;
    std::map<std::string,std::vector<std::string>> storageTiers;
    
public:
    // Add storageTier to dataCenter
    void addStorageTier(std::string dataCenter, std::string storageTier) {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            dataCenters.push_back(dataCenter);
            storageTiers[dataCenter] = std::vector<std::string>();
        } 
        if (std::find(storageTiers[dataCenter].begin(), storageTiers[dataCenter].end(), storageTier) == storageTiers[dataCenter].end()) {
            storageTiers[dataCenter].push_back(storageTier);
        }
        else {
            throw std::runtime_error("ERROR: " + storageTier + " already exists in " + dataCenter);
        }
    }

    // Get list of all Data Centers
    std::vector<std::string> getDataCenters() const {
        return dataCenters;
    }

    // Get list of all Storage Tiers in dataCenter
    std::vector<std::string> getStorageTiers(std::string dataCenter) const {
        return storageTiers.at(dataCenter);
    }

private:
    std::map<std::pair<std::string,std::string>,Cost> networkCost;
    std::map<std::pair<std::string,std::string>,Cost> storageCost;
    std::map<std::pair<std::string,std::string>,Cost> getCost, putCost, retrieveCost, writeCost;

public:
    // Set network cost between dataCenter1 and dataCenter2
    void setNetworkCost(std::string dataCenter1, std::string dataCenter2, Cost cost) {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter1) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter1 + " not found" );
        }
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter2) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter2 + " not found" );
        }
        if (networkCost.find(std::make_pair(dataCenter1, dataCenter2)) != networkCost.end()) {
            throw std::runtime_error("ERROR: Network cost already exist for " + dataCenter1 + " and " + dataCenter2);
        }

        networkCost[std::make_pair(dataCenter1, dataCenter2)] = cost;
    }

    // Get network cost betwen dataCenter1 and dataCenter2
    Cost getNetworkCost(std::string dataCenter1, std::string dataCenter2) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter1) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter1 + " not found" );
        } 
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter2) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter2 + " not found" );
        }
        if (networkCost.find(std::make_pair(dataCenter1, dataCenter2)) == networkCost.end()) {
            throw std::runtime_error("ERROR: Network cost does not exist for " + dataCenter1 + " and " + dataCenter2);
        }

        return networkCost.at(std::make_pair(dataCenter1, dataCenter2));
    }

    // Set storage cost of storageTier in dataCenter
    void setStorageCost(std::string dataCenter, std::string storageTier, Cost cost) {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers[dataCenter].begin(), storageTiers[dataCenter].end(), storageTier) == storageTiers[dataCenter].end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (storageCost.find(std::make_pair(dataCenter, storageTier)) != storageCost.end()) {
            throw std::runtime_error("ERROR: Storage cost already exist for " + storageTier + " in " + dataCenter);
        }

        storageCost[std::make_pair(dataCenter, storageTier)] = cost;
    }

    // Get storage cost of storageTier in dataCenter
    Cost getStorageCost(std::string dataCenter, std::string storageTier) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers.at(dataCenter).begin(), storageTiers.at(dataCenter).end(), storageTier) == storageTiers.at(dataCenter).end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (storageCost.find(std::make_pair(dataCenter, storageTier)) == storageCost.end()) {
            throw std::runtime_error("ERROR: Storage cost does not exist for " + storageTier + " in " + dataCenter);
        }

        return storageCost.at(std::make_pair(dataCenter, storageTier));
    }

    // Set get request cost of storageTier in dataCenter
    void setGetCost(std::string dataCenter, std::string storageTier, Cost cost) {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers[dataCenter].begin(), storageTiers[dataCenter].end(), storageTier) == storageTiers[dataCenter].end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (getCost.find(std::make_pair(dataCenter, storageTier)) != getCost.end()) {
            throw std::runtime_error("ERROR: Get request cost already exist for " + storageTier + " in " + dataCenter);
        }

        getCost[std::make_pair(dataCenter, storageTier)] = cost;
    }

    // Get get request cost of storageTier in dataCenter
    Cost getGetCost(std::string dataCenter, std::string storageTier) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers.at(dataCenter).begin(), storageTiers.at(dataCenter).end(), storageTier) == storageTiers.at(dataCenter).end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (getCost.find(std::make_pair(dataCenter, storageTier)) == getCost.end()) {
            throw std::runtime_error("ERROR: Get request cost does not exist for " + storageTier + " in " + dataCenter);
        }

        return getCost.at(std::make_pair(dataCenter, storageTier));
    }

    // Set put request cost of storageTier in dataCenter
    void setPutCost(std::string dataCenter, std::string storageTier, Cost cost) {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers[dataCenter].begin(), storageTiers[dataCenter].end(), storageTier) == storageTiers[dataCenter].end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (putCost.find(std::make_pair(dataCenter, storageTier)) != putCost.end()) {
            throw std::runtime_error("ERROR: Put request cost already exist for " + storageTier + " in " + dataCenter);
        }

        putCost[std::make_pair(dataCenter, storageTier)] = cost;
    }

    // Get put request cost of storageTier in dataCenter
    Cost getPutCost(std::string dataCenter, std::string storageTier) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers.at(dataCenter).begin(), storageTiers.at(dataCenter).end(), storageTier) == storageTiers.at(dataCenter).end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (putCost.find(std::make_pair(dataCenter, storageTier)) == putCost.end()) {
            throw std::runtime_error("ERROR: Put request cost does not exist for " + storageTier + " in " + dataCenter);
        }

        return putCost.at(std::make_pair(dataCenter, storageTier));
    }

    // Set data retrieval cost of storageTier in dataCenter
    void setRetrieveCost(std::string dataCenter, std::string storageTier, Cost cost) {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers[dataCenter].begin(), storageTiers[dataCenter].end(), storageTier) == storageTiers[dataCenter].end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (retrieveCost.find(std::make_pair(dataCenter, storageTier)) != retrieveCost.end()) {
            throw std::runtime_error("ERROR: Data retrieval cost already exist for " + storageTier + " in " + dataCenter);
        }

        retrieveCost[std::make_pair(dataCenter, storageTier)] = cost;
    }

    // Get data retrieval cost of storageTier in dataCenter
    Cost getRetrieveCost(std::string dataCenter, std::string storageTier) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers.at(dataCenter).begin(), storageTiers.at(dataCenter).end(), storageTier) == storageTiers.at(dataCenter).end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (retrieveCost.find(std::make_pair(dataCenter, storageTier)) == retrieveCost.end()) {
            throw std::runtime_error("ERROR: Data retrieval cost does not exist for " + storageTier + " in " + dataCenter);
        }

        return retrieveCost.at(std::make_pair(dataCenter, storageTier));
    }

    // Set data write cost of storageTier in dataCenter
    void setWriteCost(std::string dataCenter, std::string storageTier, Cost cost) {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers[dataCenter].begin(), storageTiers[dataCenter].end(), storageTier) == storageTiers[dataCenter].end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (writeCost.find(std::make_pair(dataCenter, storageTier)) != writeCost.end()) {
            throw std::runtime_error("ERROR: Data write cost already exist for " + storageTier + " in " + dataCenter);
        }

        writeCost[std::make_pair(dataCenter, storageTier)] = cost;
    }

    // Get data write cost of storageTier in dataCenter
    Cost getWriteCost(std::string dataCenter, std::string storageTier) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers.at(dataCenter).begin(), storageTiers.at(dataCenter).end(), storageTier) == storageTiers.at(dataCenter).end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (writeCost.find(std::make_pair(dataCenter, storageTier)) == writeCost.end()) {
            throw std::runtime_error("ERROR: Data write cost does not exist for " + storageTier + " in " + dataCenter);
        }

        return writeCost.at(std::make_pair(dataCenter, storageTier));
    }

private:
    Latency slaGet = -1;
    Latency slaPut = -1;

public:
    // Set SLA Get
    void setSLAGet(Latency sla) {
        if (slaGet != -1) {
            throw std::runtime_error("ERROR: SLA Get already exist");
        }
        slaGet = sla;
    }

    // Get SLA Get
    Latency getSLAGet() const {
        if (slaGet == -1) {
            throw std::runtime_error("ERROR: SLA Get does not exist");
        }
        return slaGet;
    }

    // Set SLA Put
    void setSLAPut(Latency sla) {
        if (slaPut != -1) {
            throw std::runtime_error("ERROR: SLA Put already exist");
        }
        slaPut = sla;
    }

    // Get SLA Put
    Latency getSLAPut() const {
        if (slaPut == -1) {
            throw std::runtime_error("ERROR: SLA Put does not exist");
        }
        return slaPut;
    }

private:
    int localeCount = -1;
    int faults = -1;

public:
    // Set Locale Count
    void setLC(int LC) {
        if (localeCount != -1) {
            throw std::runtime_error("ERROR: Locale Count already exist");
        }
        localeCount = LC;
    }

    // Get Locale Count
    int getLC() const {
        if (localeCount == -1) {
            throw std::runtime_error("ERROR: Locale Count does not exist");
        }
        return localeCount;
    }
    
    // Set minimum DC faults handled
    void setF(int F) {
        if (faults != -1) {
            throw std::runtime_error("ERROR: Minimum DC faults already exist");
        }
        faults = F;
    }

    // Get minimum DC faults handled
    int getF() const {
        if (faults == - 1) {
            throw std::runtime_error("ERROR: Minimum DC faults does not exist");
        }
        return faults;
    }

private:
    std::string center = "";

public:
    // Set central DC location
    void setCenter(std::string dataCenter) {
        if (center != "") {
            throw std::runtime_error("ERROR: Centralized DC location already exist");
        }
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data center " + dataCenter + " does not exist");
        }
        center = dataCenter;
    }

    // Get central DC location
    std::string getCenter() const {
        if (center == "") {
            throw std::runtime_error("ERROR: Centralized DC location does not exist");
        }
        return center;
    }

private:
    std::map<std::string,Size> aveSize;

public:
    // Set average object size in dataCenter
    void setSize(std::string dataCenter, Size size) {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (aveSize.find(dataCenter) != aveSize.end()) {
            throw std::runtime_error("ERROR: Average size already exist for " + dataCenter);
        }

        aveSize[dataCenter] = size;
    }

    // Get average object size in dataCenter
    Size getSize(std::string dataCenter) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (aveSize.find(dataCenter) == aveSize.end()) {
            throw std::runtime_error("ERROR: Average size does not exist for " + dataCenter);
        }

        return aveSize.at(dataCenter);
    }

private:
    std::map<std::pair<std::string,std::string>,Latency> networkLatency;
    std::map<std::pair<std::string,std::string>,Latency> getLatency, putLatency;

public:
    // Set network latency between dataCenter1 and dataCenter2
    void setNetworkLatency(std::string dataCenter1, std::string dataCenter2, Latency latency) {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter1) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter1 + " not found" );
        }
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter2) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter2 + " not found" );
        }
        if (networkLatency.find(std::make_pair(dataCenter1, dataCenter2)) != networkLatency.end()) {
            throw std::runtime_error("ERROR: Network latency already exist for " + dataCenter1 + " and " + dataCenter2);
        }

        networkLatency[std::make_pair(dataCenter1, dataCenter2)] = latency;
    }

    // Get network latency betwen dataCenter1 and dataCenter2
    Latency getNetworkLatency(std::string dataCenter1, std::string dataCenter2) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter1) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter1 + " not found" );
        } 
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter2) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter2 + " not found" );
        }
        if (networkLatency.find(std::make_pair(dataCenter1, dataCenter2)) == networkLatency.end()) {
            throw std::runtime_error("ERROR: Network latency does not exist for " + dataCenter1 + " and " + dataCenter2);
        }

        return networkLatency.at(std::make_pair(dataCenter1, dataCenter2));
    }

    // Set get latency of storageTier in dataCenter
    void setGetLatency(std::string dataCenter, std::string storageTier, Latency latency) {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers[dataCenter].begin(), storageTiers[dataCenter].end(), storageTier) == storageTiers[dataCenter].end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (getLatency.find(std::make_pair(dataCenter, storageTier)) != getLatency.end()) {
            throw std::runtime_error("ERROR: Get latency already exist for " + storageTier + " in " + dataCenter);
        }

        getLatency[std::make_pair(dataCenter, storageTier)] = latency;
    }

    // Get get latency of storageTier in dataCenter
    Cost getGetLatency(std::string dataCenter, std::string storageTier) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers.at(dataCenter).begin(), storageTiers.at(dataCenter).end(), storageTier) == storageTiers.at(dataCenter).end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (getLatency.find(std::make_pair(dataCenter, storageTier)) == getLatency.end()) {
            throw std::runtime_error("ERROR: Get latency does not exist for " + storageTier + " in " + dataCenter);
        }

        return getLatency.at(std::make_pair(dataCenter, storageTier));
    }

    // Set put latency of storageTier in dataCenter
    void setPutLatency(std::string dataCenter, std::string storageTier, Latency latency) {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers[dataCenter].begin(), storageTiers[dataCenter].end(), storageTier) == storageTiers[dataCenter].end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (putLatency.find(std::make_pair(dataCenter, storageTier)) != putLatency.end()) {
            throw std::runtime_error("ERROR: Put latency already exist for " + storageTier + " in " + dataCenter);
        }

        putLatency[std::make_pair(dataCenter, storageTier)] = latency;
    }

    // Get get latency of storageTier in dataCenter
    Cost getPutLatency(std::string dataCenter, std::string storageTier) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers.at(dataCenter).begin(), storageTiers.at(dataCenter).end(), storageTier) == storageTiers.at(dataCenter).end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }
        if (putLatency.find(std::make_pair(dataCenter, storageTier)) == putLatency.end()) {
            throw std::runtime_error("ERROR: Put latency does not exist for " + storageTier + " in " + dataCenter);
        }

        return putLatency.at(std::make_pair(dataCenter, storageTier));
    }

private:
    std::map<std::string,Request> getRequest, putRequest;

public:
    // Set number of get request in dataCenter
    void setGetRequest(std::string dataCenter, Request request) {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (getRequest.find(dataCenter) != getRequest.end()) {
            throw std::runtime_error("ERROR: Number of get request already exist for " + dataCenter);
        }

        getRequest[dataCenter] = request;
    }

    // Get number of get request in dataCenter
    Request getGetRequest(std::string dataCenter) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (getRequest.find(dataCenter) == getRequest.end()) {
            throw std::runtime_error("ERROR: Number of get request does not exist for " + dataCenter);
        }

        return getRequest.at(dataCenter);
    }
    
    // Set number of put request in dataCenter
    void setPutRequest(std::string dataCenter, Request request) {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (putRequest.find(dataCenter) != putRequest.end()) {
            throw std::runtime_error("ERROR: Number of put request already exist for " + dataCenter);
        }

        putRequest[dataCenter] = request;
    }

    // Get number of put request in dataCenter
    Request getPutRequest(std::string dataCenter) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (putRequest.find(dataCenter) == putRequest.end()) {
            throw std::runtime_error("ERROR: Number of put request does not exist for " + dataCenter);
        }

        return putRequest.at(dataCenter);
    }

private:
    std::map<std::pair<std::string,std::string>,int> storageToIdx;
    std::vector<std::pair<std::string,std::string>> idxToStorage;
    std::map<std::string,int> dataToIdx;

public:
    // Mapping Data Centers and Storage Tiers to int
    void update() {
        storageToIdx.clear();
        idxToStorage.clear();
        dataToIdx.clear();
        int idx = 0;

        for (std::string dataCenter: dataCenters) {
            for (std::string storageTier: storageTiers[dataCenter]) {
                storageToIdx[std::make_pair(dataCenter, storageTier)] = idxToStorage.size();
                idxToStorage.push_back(std::make_pair(dataCenter, storageTier));
            }
            dataToIdx[dataCenter] = idx;
            idx ++;
        }
    }

    // Get number of Data Centers
    int getNumDataCenters() const {
        return dataCenters.size();
    }

    // Get index of dataCenter in list of Data Centers
    int getIdxDataCenters(std::string dataCenter) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        return dataToIdx.at(dataCenter);
    }

    // Get the idx-th Data Center
    std::string getDataCenters(int idx) const {
        if (idx >= getNumDataCenters()) {
            throw std::runtime_error("ERROR: index should be less than " + std::to_string(getNumDataCenters()));
        }
        return dataCenters[idx];
    }

    // Get number of (dataCenter, storageTier) pairs
    int getNumStorageTiers() const {
        return idxToStorage.size();
    }

    // Get number of Storage Tiers in dataCenter
    int getNumStorageTiers(std::string dataCenter) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        return storageTiers.at(dataCenter).size();
    }

    // Get number of Storage Tiers in idx-th Data Center
    int getNumStorageTiers(int idx) const {
        if (idx >= getNumDataCenters()) {
            throw std::runtime_error("ERROR: index should be less than " + std::to_string(getNumDataCenters()));
        }
        return getNumStorageTiers(dataCenters[idx]);
    }

    // Get the index of (dataCenter, storageTier)
    int getIdxStorageTiers(std::string dataCenter, std::string storageTier, std::string option) const {
        if (std::find(dataCenters.begin(), dataCenters.end(), dataCenter) == dataCenters.end()) {
            throw std::runtime_error("ERROR: Data Center " + dataCenter + " not found" );
        }
        if (std::find(storageTiers.at(dataCenter).begin(), storageTiers.at(dataCenter).end(), storageTier) == storageTiers.at(dataCenter).end()) {
            throw std::runtime_error("ERROR: " + storageTier + " does not exists in " + dataCenter);
        }

        if (option == "all") return storageToIdx.at(std::make_pair(dataCenter, storageTier));
        if (option == "dataCenter") {
            return find(storageTiers.at(dataCenter).begin(), storageTiers.at(dataCenter).end(), storageTier) - storageTiers.at(dataCenter).begin();
        }

        throw std::runtime_error("ERROR: Invalid option parameter");

    }

    // Get the Storage Tier and Data Center of idx-th (dataCenter, storageTier)
    std::string getStorageTiers(int idx, std::string option) const {
        if (idx >= getNumStorageTiers()) {
            throw std::runtime_error("ERROR: index should be less than " + std::to_string(getNumStorageTiers()));
        }

        if (option == "dataCenter") return idxToStorage[idx].first;
        if (option == "storageTier") return idxToStorage[idx].second;

        throw std::runtime_error("ERROR: Invalid option parameter");
    }

public:
    // Check if all information required are present
    void checkAll() const {
        for (std::string dataCenter1: dataCenters) {
            getSize(dataCenter1);
            getGetRequest(dataCenter1);
            getPutRequest(dataCenter1);

            for (std::string storageTier: storageTiers.at(dataCenter1)) {
                getStorageCost(dataCenter1, storageTier);
                getGetCost(dataCenter1, storageTier);
                getPutCost(dataCenter1, storageTier);
                getRetrieveCost(dataCenter1, storageTier);
                getWriteCost(dataCenter1, storageTier);
                getGetLatency(dataCenter1, storageTier);
                getPutLatency(dataCenter1, storageTier);
            }

            for (std::string dataCenter2: dataCenters) {
                getNetworkCost(dataCenter1, dataCenter2);
                getNetworkLatency(dataCenter1, dataCenter2);
            }
        }

        getCenter();
        getSLAGet();
        getSLAPut();
        getLC();
        getF();
    }

    // Set random gdss instance
    void setInstance(std::vector<int> const& dcList = std::vector<int>{2,2}) {
        dataCenters.clear();
        storageTiers.clear();
        
        // Add storage tiers
        for (int i = 0; i < dcList.size(); ++i) {
            for (int j = 0; j < dcList[i]; ++j) {
                addStorageTier("DC" + std::to_string(i + 1), "ST" + std::to_string(i + 1) + "_" + std::to_string(j + 1));
            }
        }
        update();

        // Initialize random number generators
        srand((unsigned) time(NULL));
        std::default_random_engine generator;
        generator.seed((unsigned) time(NULL));
        std::uniform_real_distribution<long double> distribution(0.0, 2.0);

        std::set<std::string> dataCentersUsed;
        for (std::string dataCenter1: dataCenters) {
            setSize(dataCenter1, rand() % 10);
            setGetRequest(dataCenter1, rand() % 5);
            setPutRequest(dataCenter1, rand() % 5);

            for (std::string storageTier: storageTiers.at(dataCenter1)) {
                setStorageCost(dataCenter1, storageTier, distribution(generator));
                setGetCost(dataCenter1, storageTier, distribution(generator));
                setPutCost(dataCenter1, storageTier, distribution(generator));
                setRetrieveCost(dataCenter1, storageTier, distribution(generator));
                setWriteCost(dataCenter1, storageTier, distribution(generator));
                setGetLatency(dataCenter1, storageTier, distribution(generator));
                setPutLatency(dataCenter1, storageTier, distribution(generator));
            }

            for (std::string dataCenter2: dataCenters) {
                if (dataCentersUsed.count(dataCenter2) and dataCenter1 == dataCenter2) continue;
                setNetworkCost(dataCenter1, dataCenter2, distribution(generator));
                setNetworkLatency(dataCenter1, dataCenter2, distribution(generator));
            }

            dataCentersUsed.insert(dataCenter1);
        }
        
        setCenter(dataCenters[rand() % getNumDataCenters()]);
        setSLAGet(3.5);
        setSLAPut(3.5);
        setLC(std::ceil(getNumDataCenters() / 2));
        setF(getNumDataCenters() / 2 - 1);
    }

    // Read information from JSON file
    void readJSON(std::string cost_info, std::string monitoring_info, std::string query, std::string goals) {
        dataCenters.clear();
        storageTiers.clear();

        // Cost information
        std::ifstream f(cost_info);
        json cost_data = json::parse(f);
        for (auto& regions : cost_data.items()) {
            json region = regions.value();
            for (auto& storages : region["storage_cost"].items()) {
                json storage = storages.value();
                addStorageTier(regions.key(), storages.key());
                setStorageCost(regions.key(), storages.key(), storage["storage_cost"].get<Cost>());
                setGetCost(regions.key(), storages.key(), storage["get_request_cost"].get<Cost>());
                setPutCost(regions.key(), storages.key(), storage["put_request_cost"].get<Cost>());
                setRetrieveCost(regions.key(), storages.key(), storage["data_retrieval"].get<Cost>());
                setWriteCost(regions.key(), storages.key(), storage["data_write"].get<Cost>());
            }
        }
        for (auto& regions : cost_data.items()) {
            json region = regions.value();
            for (auto& networks : region["network_cost"].items()) {
                json network = networks.value();
                setNetworkCost(regions.key(), networks.key(), network.get<Cost>());
            }
        }

        // Latency information
        std::ifstream g(monitoring_info);
        json monitoring_data = json::parse(g);
        for (auto& regions : monitoring_data.items()) {
            json region = regions.value();
            for (auto& networks : region["network_latency"].items()) {
                json network = networks.value();
                setNetworkLatency(regions.key(), networks.key(), network.get<Latency>());
            }
            for (auto& storages : region["storage_latency"].items()) {
                json storage = storages.value();
                setPutLatency(regions.key(), storages.key(), storage["put_latency"].get<Latency>());
                setGetLatency(regions.key(), storages.key(), storage["get_latency"].get<Latency>());
            }
        }

        // Size/Request information
        std::ifstream h(query);
        json query_data = json::parse(h);
        for (auto& dataCenter : dataCenters) {
            setSize(dataCenter, query_data["object_size"].get<Size>());
        }
        for (auto& regions : query_data["access_info"].items()) {
            json region = regions.value();
            setGetRequest(regions.key(), region["get_access_cnt"].get<Request>());
            setPutRequest(regions.key(), region["put_access_cnt"].get<Request>());
        }

        // Goal information
        std::ifstream i(goals);
        json goals_data = json::parse(i);
        setCenter(goals_data["center"].get<std::string>());
        setSLAGet(goals_data["get_sla"].get<Latency>());
        setSLAPut(goals_data["put_sla"].get<Latency>());
        setLC(goals_data["lc"].get<int>());
        setF(goals_data["degree_of_fault"].get<int>());

        update();
    }

};