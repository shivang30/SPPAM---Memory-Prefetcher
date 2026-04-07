#ifndef PREFETCHER_NEXT_LINE_H
#define PREFETCHER_NEXT_LINE_H

#include <cstdint>

#include "address.h"
#include "modules.h"

struct sppam : public champsim::modules::prefetcher {
  using prefetcher::prefetcher;
  uint32_t prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type,
                                    uint32_t metadata_in);
  // This function is called when a tag is checked in the cache. The parameters passed are:
  // addr: the address of the packet. If this is the first-level cache, the offset bits are included. Otherwise, the offset bits are zero. If the cache was
  // configured with “virtual_prefetch”: true, this address will be a virtual address. Otherwise, this is a physical address. ip: the address of the instruction
  // that initiated the demand. If the packet is a prefetch from another level, this value will be 0. cache_hit: if this tag check is a hit, this value is
  // nonzero. Otherwise, it is 0. useful_prefetch: if this tag check hit a prior prefetch, this value is true. type: the result of
  // static_cast<std::underlying_type_t<access_type>>(v) for v in: * access_type::LOAD * access_type::RFO * access_type::PREFETCH * access_type::WRITE *
  // access_type::TRANSLATION metadata_in: the metadata carried along by the packet. The function should return metadata that will be stored alongside the
  // block.

  uint32_t prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, champsim::address evicted_addr, uint32_t metadata_in);
  // This function is called when a miss is filled in the cache. The parameters passed are:
  // addr: the address of the packet. If this is the first-level cache, the offset bits are included. Otherwise, the offset bits are zero. If the cache was
  // configured with “virtual_prefetch”: true, this address will be a virtual address. Otherwise, this is a physical address. set: the set that the fill
  // occurred in way: the way that the fill occurred in, or this->NUM_WAY if a bypass occurred prefetch: if this tag check hit a prior prefetch, this value is
  // true. metadata_in: the metadata carried along by the packet. The function should return metadata that will be stored alongside the block.
  void prefetcher_initialize();
  // This function is called when the cache is initialized.
  // You can use it to initialize elements of dynamic structures, such as std::vector or std::map.

  // void prefetcher_branch_operate(champsim::address ip, uint8_t branch_type, champsim::address branch_target) {}
  // void prefetcher_cycle_operate() {}
  void prefetcher_final_stats() {} // This function is called at the end of the simulation and can be used to print statistics.

  struct Sig_row {
    uint64_t tag=0;
    uint16_t sig=0;
    uint16_t last_offset=0;
    uint16_t lru_last_used=0; // 0 to 255
    bool valid=false;
  };


  class signature_table
  {
  public:
    Sig_row Sig_table[256];

    signature_table() // constructor
    {
      for (uint32_t i = 0; i < 255; i++) {
        Sig_table->valid = false;
        Sig_table->sig = 0;
        Sig_table->lru_last_used = 0;
      }
    }
  };

  
  struct Pattern_Table_entry {
    struct Deltas {
      int8_t delta = 0;
      uint8_t c_delta = 0;
    } predictors[4];
    uint8_t c_sig = 0;
  };

  class pattern_table
  {
  public:
    Pattern_Table_entry pattern[512]; // 12 bit signature

  protected:
  };
  
  
  void  update_pt(uint16_t sig, int8_t delta);
  void prefetch_gen();
  signature_table s_table;
  pattern_table p_table;
};

#endif
