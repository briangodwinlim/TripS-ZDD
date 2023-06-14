# Storage Switch System via Zero-Suppressed Binary Decision Diagram

A zero-suppressed binary decision diagram (ZDD) implementation of the Storage Switch System (TripS) for Geo-Distributed Multi-Cloud Data Center Storage Tiering and Selection Problem.

## Usage

### Build

```
make
```

### Run

```
./trips-zdd [ <cost_info> <monitoring_info> <query> <goals> ] [ <options>... ]
```

#### Example 1

Create a random geo-distributed storage system instance with 3 data centers each having 4 storage tiers. Get the first 3 optimal data placements.

```
./trips-zdd -dcList -getconfig 3 <<< "4 4 4"
```

#### Example 2

Read a geo-distributed storage system instance from JSON files with strong consistency in latency SLA constraint. Use openMP during ZDD construction.

```
OMP_NUM_THREADS=4 ./trips-zdd data/cost_info data/monitoring_info data/query data/goals -strongSLA -openMP
```

## Related Repositories

- [TdZdd](https://github.com/kunisura/TdZdd/)
- [TdZdd_Utility](https://github.com/briangodwinlim/TdZdd_Utility)
- [SAPPOROBDD](https://github.com/Shin-ichi-Minato/SAPPOROBDD)
- [sbdd_helper](https://github.com/junkawahara/sbdd_helper)
- [json](https://github.com/nlohmann/json)

## Reference

- [TripS: Automated Multi-tiered Data Placement in a Geo-distributed Cloud Environment](https://dl.acm.org/doi/pdf/10.1145/3078468.3078485)
