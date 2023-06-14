/*
 * A DdSpec for generating all valid configurations/placements for the 
 * Geo-Distributed Multi-Cloud Data Center Storage Tiering and Selection Problem
 */

#pragma once

#include <tdzdd/DdSpec.hpp>

#include "GeoDistributedStorageSystem.hpp"

namespace tdzdd {

union ValidConfigMate {
private:
    uint16_t hash;          ///< hash for T_{jkt}
    int16_t faults;         ///< minimum number of DC faults

public:
    // Initialize faults
    ValidConfigMate(int faults)
        : faults(faults) {
    }

    bool operator==(ValidConfigMate const& o) const {
        return this == &o;
    }

    bool operator!=(ValidConfigMate const& o) const {
        return this != &o;
    }

    friend class ValidConfig;
};

class ValidConfig: public PodArrayDdSpec<ValidConfig,ValidConfigMate,2> {
    typedef ValidConfigMate Mate;

    GeoDistributedStorageSystem const& gdss;
    std::string const slaOption;
    int const localeCount;
    int const cellSize;
    int const numCells;
    int const Pwidth;
    int const Twidth;
    int const faults;
    int const numDC;
    int const numST;
    int const n;

    int pow(int exp) const {
        if (exp == 0) return 1;
        else if (exp % 2 == 0) return pow(exp / 2) * pow(exp / 2);
        else return 3 * pow(exp - 1);
    }

    bool setTHash(Mate* mate, int j, int k, int val = 0) const {
        assert (0 <= j && j < numDC);
        assert (0 <= k && k < numDC);
        assert (0 <= val && val < 3);
        
        if (getTHash(mate, j, k) != 0 && getTHash(mate, j, k) != val) return false;
        if (getTHash(mate, j, k) == 0) {
            if (getLC(mate, j) == localeCount && val == 2) return false;    // Exactly LC
            int t = j + numDC * k;
            mate[t / cellSize].hash += val * pow(t % cellSize);
        }
        return true;
    }

    int getTHash(Mate const* mate, int j, int k) const { 
        assert (0 <= j && j < numDC);
        assert (0 <= k && k < numDC);

        int t = j + numDC * k;
        return mate[t / cellSize].hash / pow(t % cellSize) % 3;
    }

    int getLC(Mate const* mate, int j) const {
        int LC = 0;
        for (int k = 0; k < numDC; ++k) {
            if (getTHash(mate, j, k) == 2) ++LC;
        }
        return LC;
    }

    bool doTakeP(Mate* mate, int k, int t) const {
        // if (mate[numCells].faults == 0) return false         // Exactly F + 1
        if (mate[numCells].faults) mate[numCells].faults -= 1;
        return true;
    }

    bool doNotTakeP(Mate* mate, int k, int t) const {
        // If last ST on DC, force Tjkt = 0 for all j except when forced Tjkt = 1
        std::string DCk = gdss.getStorageTiers(t, "dataCenter");
        std::string STt = gdss.getStorageTiers(t, "storageTier");
        int idx = gdss.getIdxStorageTiers(DCk, STt, "dataCenter");
        if (gdss.getNumStorageTiers(k) == idx + 1) {
            for (int j = 0; j < numDC; ++j) {
                if (!setTHash(mate, j, k, 1)) return false;
            }
        }
        return true;
    }

    bool SLAConstraint(int j, int k, int t) const {
        std::string DCj = gdss.getDataCenters(j);
        std::string DCk = gdss.getStorageTiers(t, "dataCenter");
        std::string STt = gdss.getStorageTiers(t, "storageTier");

        if (slaOption == "eventual") {
            if (gdss.getNetworkLatency(DCj, DCk) + gdss.getGetLatency(DCk, STt) > gdss.getSLAGet()) return false;
            if (gdss.getNetworkLatency(DCj, DCk) + gdss.getPutLatency(DCk, STt) > gdss.getSLAPut()) return false;
        }
        if (slaOption == "strong") {
            if (gdss.getNetworkLatency(DCj, DCk) + gdss.getGetLatency(DCk, STt) + 2 * gdss.getNetworkLatency(DCk, gdss.getCenter()) > gdss.getSLAGet()) return false;
            GeoDistributedStorageSystem::Latency maxNetworkLatency = 0;
            for (std::string dataCenter: gdss.getDataCenters()) maxNetworkLatency = std::max(maxNetworkLatency, gdss.getNetworkLatency(DCk, dataCenter));
            if (gdss.getNetworkLatency(DCj, DCk) + gdss.getPutLatency(DCk, STt) + 2 * gdss.getNetworkLatency(DCk, gdss.getCenter()) + maxNetworkLatency > gdss.getSLAPut()) return false;
        }
        return true;
    }

    bool doTakeT(Mate* mate, int j, int k, int t) const {
        if (!SLAConstraint(j, k, t)) return false;
        if (!setTHash(mate, j, k, 2)) return false;
        return true;
    }

    bool doNotTakeT(Mate* mate, int j, int k, int t) const {
        if (!setTHash(mate, j, k, 1)) return false;
        return true;
    }

    bool doTakeB(Mate* mate, int i, int j, int k, int t) const {
        if (j == k) return false;
        if (!setTHash(mate, i, j, 2)) return false;
        return true;
    }

    bool doNotTakeB(Mate* mate, int i, int j, int k, int t) const {
        if (j == k) return true;
        if (!setTHash(mate, i, j, 1)) return false;
        return true;
    }

    // Lookahead to check if possible to continue
    bool lookaheadCheck(Mate const* mate, int k) const {
        for (int j = 0; j < numDC; ++j) {
            if (getLC(mate, j) + (numDC - k) < localeCount) return false;
        }
        if (mate[numCells].faults - (numDC - k) > 0) return false;
        return true;
    }

    // Check the constraints for Locale Count and Minimum DC Fault
    bool constraintsCheck(Mate const* mate) const {
        for (int j = 0; j < numDC; ++j) {
            if (getLC(mate, j) < localeCount) return false;
        }
        if (mate[numCells].faults > 0) return false;
        return true;
    }

    // Get level of next DC
    int nextDC(int i, int j, int k, int t) const {
        if (i + 1 == numDC && j + 1 == numDC) {
            std::string DCk = gdss.getStorageTiers(t, "dataCenter");
            std::string STt = gdss.getStorageTiers(t, "storageTier");
            int idx = gdss.getIdxStorageTiers(DCk, STt, "dataCenter");
            return (gdss.getNumStorageTiers(k) - idx - 1) * Pwidth + 1;
        }
        return 0;
    }

public:
    ValidConfig(GeoDistributedStorageSystem const& gdss, std::string slaOption)
            : gdss(gdss), slaOption(slaOption),
              numDC(gdss.getNumDataCenters()), numST(gdss.getNumStorageTiers()),
              Pwidth(1 + gdss.getNumDataCenters() + gdss.getNumDataCenters() * gdss.getNumDataCenters()),
              Twidth(1 + gdss.getNumDataCenters()), 
              n(gdss.getNumStorageTiers() * (1 + gdss.getNumDataCenters() + gdss.getNumDataCenters() * gdss.getNumDataCenters())),
              cellSize(10),     // log_3 (2^16-1)
              numCells((gdss.getNumDataCenters() * gdss.getNumDataCenters() - 1) / cellSize + 1),
              localeCount(gdss.getLC()), faults(gdss.getF()) {
            this->setArraySize(numCells + 1);
    }

    int getRoot(Mate* mate) const {
        for (int i = 0; i < numCells; ++i) mate[i].hash = 0;
        mate[numCells] = Mate(faults + 1);

        return n;
    }

    int getChild(Mate* mate, int level, int take) const {
        assert(1 <= level && level <= n);
        int invLevel = n - level;

        // P_{kt}
        if (invLevel % Pwidth == 0) {
            int t = invLevel / Pwidth;
            int k = gdss.getIdxDataCenters(gdss.getStorageTiers(t, "dataCenter"));

            if (!lookaheadCheck(mate, k)) return 0;
            if (take) {
                if (!doTakeP(mate, k, t)) return 0;
            }
            else {
                if (!doNotTakeP(mate, k, t)) return 0;
                if (level == Pwidth) return constraintsCheck(mate) ? -1 : 0;
                return level - Pwidth;
            }
        }
        // T_{jkt}
        else if ((invLevel % Pwidth - 1) % Twidth == 0) {
            int t = invLevel / Pwidth;
            int k = gdss.getIdxDataCenters(gdss.getStorageTiers(t, "dataCenter"));
            int j = (invLevel % Pwidth - 1) / Twidth;

            if (take) {
                if (!doTakeT(mate, j, k, t)) return 0;
            }
            else {
                if (!doNotTakeT(mate, j, k, t)) return 0;
            }

        }
        // B_{ijkt}
        else {
            int t = invLevel / Pwidth;
            int k = gdss.getIdxDataCenters(gdss.getStorageTiers(t, "dataCenter"));
            int j = (invLevel % Pwidth - 1) / Twidth;
            int i = (invLevel % Pwidth - 1) % Twidth - 1;
            
            if (take) {
                if (!doTakeB(mate, i, j, k, t)) return 0;
            }
            else {
                if (!doNotTakeB(mate, i, j, k, t)) return 0;
            }

            if (nextDC(i, j, k, t)) {
                if (level == nextDC(i, j, k, t)) return constraintsCheck(mate) ? -1 : 0;
                return level - nextDC(i, j, k, t);
            }
        }

        if (++invLevel == n) return constraintsCheck(mate) ? -1 : 0;
        
        assert(invLevel < n);
        return n - invLevel;
    }

    int numVariables() {
        return n;
    }
};

} // namespace tdzdd